use binding::JsClass;
use filter::Filter;
use frame::Frame;
use genet_napi::{
    napi::{
        CallbackInfo, Env, HandleScope, PropertyAttributes, PropertyDescriptor, Result, Status,
        Value, ValueRef,
    },
    uv,
};
use libc;
use profile::Profile;
use serde_json;
use session::{Callback, Event, Session};
use std::{
    cmp,
    error::Error,
    ffi::{CStr, CString},
    ptr,
    rc::Rc,
    str,
    sync::{Arc, Mutex},
};

#[derive(Clone)]
struct SessionCallback {
    asyn: Arc<uv::Async>,
    events: Arc<Mutex<Vec<String>>>,
}

impl Callback for SessionCallback {
    fn on_event(&self, event: Event) {
        let json = serde_json::to_string(&event).unwrap();
        self.events.lock().unwrap().push(json);
        self.asyn.send();
    }
}

impl SessionCallback {
    fn new(env: &Env, callback: Rc<ValueRef>) -> SessionCallback {
        let ev = Arc::new(Mutex::new(Vec::<String>::new()));
        let events = ev.clone();
        SessionCallback {
            asyn: Arc::new(uv::Async::new(move || {
                let _scope = HandleScope::new(env);
                if let Some(json) = ev.lock().unwrap().pop() {
                    let argv = vec![env.create_string(&json).unwrap()];
                    let _ = env.call_function(&callback, &callback, &argv);
                }
            })),
            events,
        }
    }
}

pub fn init(env: &Env, exports: &Value) -> Result<()> {
    fn profile_ctor<'env>(env: &'env Env, info: &'env CallbackInfo) -> Result<&'env Value> {
        let profile = Profile::new();
        env.wrap(info.this(), profile)?;
        env.get_null()
    }

    fn session_ctor<'env>(env: &'env Env, info: &'env CallbackInfo) -> Result<&'env Value> {
        let profile_class = env
            .get_constructor(JsClass::SessionProfile as usize)
            .unwrap();
        if let Some([profile, callback]) = info.argv().get(0..2) {
            if env.instanceof(profile, &profile_class)? {
                let profile = env.unwrap::<Profile>(profile)?.clone();
                let callback = env.create_ref(callback);
                let session = Session::new(profile, SessionCallback::new(env, callback));
                env.wrap(info.this(), session)?;
                return env.get_null();
            }
        }
        Err(Status::InvalidArg)
    }

    fn profile_set_config<'env>(env: &'env Env, info: &'env CallbackInfo) -> Result<&'env Value> {
        let profile = env.unwrap::<Profile>(info.this())?;
        if let Some([key, value]) = info.argv().get(0..2) {
            profile.set_config(&env.get_value_string(key)?, &env.get_value_string(value)?);
            env.get_null()
        } else {
            Err(Status::InvalidArg)
        }
    }

    fn profile_load_library<'env>(env: &'env Env, info: &'env CallbackInfo) -> Result<&'env Value> {
        let profile = env.unwrap::<Profile>(info.this())?;
        if let Some(value) = info.argv().get(0) {
            if let Err(err) = profile.load_library(&env.get_value_string(value)?) {
                env.throw_error("load_library", &err.to_string())?;
            }
            env.get_null()
        } else {
            Err(Status::InvalidArg)
        }
    }

    fn profile_concurrency<'env>(env: &'env Env, info: &'env CallbackInfo) -> Result<&'env Value> {
        let profile = env.unwrap::<Profile>(info.this())?;
        if let Some(value) = info.argv().get(0) {
            profile.set_concurrency(env.get_value_uint32(value)?);
            env.get_null()
        } else {
            env.create_uint32(profile.concurrency())
        }
    }

    fn session_frames<'env>(env: &'env Env, info: &'env CallbackInfo) -> Result<&'env Value> {
        let session = env.unwrap::<Session>(info.this())?;
        if let Some([start, end]) = info.argv().get(0..2) {
            let start = env.get_value_uint32(start)?;
            let end = env.get_value_uint32(end)?;
            let frames = session.frames(start as usize..end as usize);
            let frame_class = env.get_constructor(JsClass::Frame as usize).unwrap();
            let array = env.create_array(frames.len())?;
            for i in 0..frames.len() {
                let instance = env.new_instance(&frame_class, &[])?;
                env.wrap_ptr(instance, frames[i])?;
                env.set_element(array, i as u32, instance)?;
            }
            Ok(array)
        } else {
            Err(Status::InvalidArg)
        }
    }

    fn session_filtered_frames<'env>(
        env: &'env Env,
        info: &'env CallbackInfo,
    ) -> Result<&'env Value> {
        let session = env.unwrap::<Session>(info.this())?;
        if let Some([id, start, end]) = info.argv().get(0..3) {
            let id = env.get_value_uint32(id)?;
            let start = env.get_value_uint32(start)?;
            let end = env.get_value_uint32(end)?;
            let frames = session.filtered_frames(id, start as usize..end as usize);
            let array = env.create_array(frames.len())?;
            for i in 0..frames.len() {
                env.set_element(array, i as u32, env.create_uint32(frames[i])?)?;
            }
            Ok(array)
        } else {
            Err(Status::InvalidArg)
        }
    }

    fn session_set_filter<'env>(env: &'env Env, info: &'env CallbackInfo) -> Result<&'env Value> {
        let session = env.unwrap::<Session>(info.this())?;
        if let Some([id, filter]) = info.argv().get(0..2) {
            let filter = env.get_value_string(filter)?;
            session.set_filter(
                env.get_value_uint32(id)?,
                if filter.is_empty() {
                    None
                } else {
                    Filter::compile(&filter).ok()
                },
            );
            env.get_null()
        } else {
            Err(Status::InvalidArg)
        }
    }

    fn session_create_reader<'env>(
        env: &'env Env,
        info: &'env CallbackInfo,
    ) -> Result<&'env Value> {
        let session = env.unwrap::<Session>(info.this())?;
        if let Some([id, arg]) = info.argv().get(0..2) {
            session.create_reader(&env.get_value_string(id)?, &env.get_value_string(arg)?);
            env.get_null()
        } else {
            Err(Status::InvalidArg)
        }
    }

    fn session_create_writer<'env>(
        env: &'env Env,
        info: &'env CallbackInfo,
    ) -> Result<&'env Value> {
        let session = env.unwrap::<Session>(info.this())?;
        if let Some([id, arg, filter]) = info.argv().get(0..3) {
            let filter = env.get_value_string(filter)?;
            session.create_writer(
                &env.get_value_string(id)?,
                &env.get_value_string(arg)?,
                if filter.is_empty() {
                    None
                } else {
                    Filter::compile(&filter).ok()
                },
            );
            env.get_null()
        } else {
            Err(Status::InvalidArg)
        }
    }

    fn session_close_reader<'env>(env: &'env Env, info: &'env CallbackInfo) -> Result<&'env Value> {
        let session = env.unwrap::<Session>(info.this())?;
        if let Some(value) = info.argv().get(0) {
            session.close_reader(env.get_value_uint32(value)?);
            env.get_null()
        } else {
            Err(Status::InvalidArg)
        }
    }

    fn session_length<'env>(env: &'env Env, info: &'env CallbackInfo) -> Result<&'env Value> {
        let session = env.unwrap::<Session>(info.this())?;
        env.create_uint32(session.len() as u32)
    }

    let session_class = env.define_class(
        "Session",
        session_ctor,
        &[
            PropertyDescriptor::new_method(
                env,
                "frames",
                PropertyAttributes::Default,
                session_frames,
            ),
            PropertyDescriptor::new_method(
                env,
                "filteredFrames",
                PropertyAttributes::Default,
                session_filtered_frames,
            ),
            PropertyDescriptor::new_method(
                env,
                "setFilter",
                PropertyAttributes::Default,
                session_set_filter,
            ),
            PropertyDescriptor::new_method(
                env,
                "createReader",
                PropertyAttributes::Default,
                session_create_reader,
            ),
            PropertyDescriptor::new_method(
                env,
                "createWriter",
                PropertyAttributes::Default,
                session_create_writer,
            ),
            PropertyDescriptor::new_method(
                env,
                "closeReader",
                PropertyAttributes::Default,
                session_close_reader,
            ),
            PropertyDescriptor::new_property(
                env,
                "length",
                PropertyAttributes::Default,
                session_length,
                true,
            ),
        ],
    )?;

    let profile_class = env.define_class(
        "Profile",
        profile_ctor,
        &[
            PropertyDescriptor::new_method(
                env,
                "setConfig",
                PropertyAttributes::Default,
                profile_set_config,
            ),
            PropertyDescriptor::new_method(
                env,
                "loadLibrary",
                PropertyAttributes::Default,
                profile_load_library,
            ),
            PropertyDescriptor::new_property(
                env,
                "concurrency",
                PropertyAttributes::Default,
                profile_concurrency,
                true,
            ),
        ],
    )?;
    env.set_named_property(exports, "Session", session_class)?;
    env.set_named_property(session_class, "Profile", profile_class)?;
    env.set_constructor(
        JsClass::SessionProfile as usize,
        &env.create_ref(profile_class),
    );
    Ok(())
}
