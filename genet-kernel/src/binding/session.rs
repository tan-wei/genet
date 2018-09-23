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

#[no_mangle]
pub unsafe extern "C" fn genet_session_profile_new() -> *mut Profile {
    Box::into_raw(Box::new(Profile::new()))
}

#[no_mangle]
pub unsafe extern "C" fn genet_session_profile_concurrency(profile: *const Profile) -> u32 {
    (*profile).concurrency()
}

#[no_mangle]
pub unsafe extern "C" fn genet_session_profile_set_concurrency(
    profile: *mut Profile,
    concurrency: u32,
) {
    (*profile).set_concurrency(concurrency)
}

#[no_mangle]
pub unsafe extern "C" fn genet_session_profile_load_library(
    profile: *mut Profile,
    path: *const libc::c_char,
) -> *mut libc::c_char {
    let path = str::from_utf8_unchecked(CStr::from_ptr(path).to_bytes());
    if let Err(err) = (*profile).load_library(path) {
        CString::new(err.description()).unwrap().into_raw()
    } else {
        ptr::null_mut()
    }
}

#[no_mangle]
pub unsafe extern "C" fn genet_session_profile_set_config(
    profile: *mut Profile,
    key: *const libc::c_char,
    value: *const libc::c_char,
) {
    let key = str::from_utf8_unchecked(CStr::from_ptr(key).to_bytes());
    let value = str::from_utf8_unchecked(CStr::from_ptr(value).to_bytes());
    (*profile).set_config(key, value);
}

#[no_mangle]
pub unsafe extern "C" fn genet_session_profile_free(profile: *mut Profile) {
    if !profile.is_null() {
        Box::from_raw(profile);
    }
}

#[derive(Clone, Copy)]
struct DataHolder {
    callback: extern "C" fn(*mut libc::c_void, *mut libc::c_char),
    data: *mut libc::c_void,
}

unsafe impl Sync for DataHolder {}
unsafe impl Send for DataHolder {}

impl Callback for DataHolder {
    fn on_event(&self, event: Event) {
        let json = CString::new(serde_json::to_string(&event).unwrap())
            .unwrap()
            .into_raw();
        (self.callback)(self.data, json);
    }
}

#[no_mangle]
pub unsafe extern "C" fn genet_session_new(
    profile: *const Profile,
    callback: extern "C" fn(*mut libc::c_void, *mut libc::c_char),
    data: *mut libc::c_void,
) -> *mut Session {
    let holder = DataHolder { callback, data };
    Box::into_raw(Box::new(Session::new((*profile).clone(), holder)))
}

#[no_mangle]
pub unsafe extern "C" fn genet_session_frames(
    session: *const Session,
    start: u32,
    end: u32,
    len: *mut u32,
    dst: *mut *const Frame,
) {
    let frames = (*session).frames(start as usize..end as usize);
    let size = cmp::min(*len as usize, frames.len());
    if size > 0 {
        ptr::copy_nonoverlapping(frames.as_ptr(), dst, size);
    }
    *len = size as u32;
}

#[no_mangle]
pub unsafe extern "C" fn genet_session_filtered_frames(
    session: *const Session,
    id: u32,
    start: u32,
    end: u32,
    len: *mut u32,
    dst: *mut u32,
) {
    let frames = (*session).filtered_frames(id, start as usize..end as usize);
    let size = cmp::min(*len as usize, frames.len());
    if size > 0 {
        ptr::copy_nonoverlapping(frames.as_ptr(), dst, size);
    }
    *len = size as u32;
}

#[no_mangle]
pub unsafe extern "C" fn genet_session_create_reader(
    session: *mut Session,
    id: *const libc::c_char,
    arg: *const libc::c_char,
) -> u32 {
    let id = str::from_utf8_unchecked(CStr::from_ptr(id).to_bytes());
    let arg = str::from_utf8_unchecked(CStr::from_ptr(arg).to_bytes());
    (*session).create_reader(id, arg)
}

#[no_mangle]
pub unsafe extern "C" fn genet_session_create_writer(
    session: *mut Session,
    id: *const libc::c_char,
    arg: *const libc::c_char,
    filter: *const libc::c_char,
) -> u32 {
    let id = str::from_utf8_unchecked(CStr::from_ptr(id).to_bytes());
    let arg = str::from_utf8_unchecked(CStr::from_ptr(arg).to_bytes());
    let filter = str::from_utf8_unchecked(CStr::from_ptr(filter).to_bytes());
    (*session).create_writer(
        id,
        arg,
        if filter.is_empty() {
            None
        } else {
            Filter::compile(filter).ok()
        },
    )
}

#[no_mangle]
pub unsafe extern "C" fn genet_session_close_reader(session: *mut Session, handle: u32) {
    (*session).close_reader(handle);
}
#[no_mangle]
pub unsafe extern "C" fn genet_session_set_filter(
    session: *mut Session,
    id: u32,
    filter: *const libc::c_char,
) {
    let filter = str::from_utf8_unchecked(CStr::from_ptr(filter).to_bytes());
    (*session).set_filter(
        id,
        if filter.is_empty() {
            None
        } else {
            Filter::compile(filter).ok()
        },
    );
}

#[no_mangle]
pub unsafe extern "C" fn genet_session_len(session: *const Session) -> u32 {
    (*session).len() as u32
}

#[no_mangle]
pub unsafe extern "C" fn genet_session_free(session: *mut Session) {
    if !session.is_null() {
        Box::from_raw(session);
    }
}

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

pub fn init(env: &mut Env, exports: &mut Value) -> Result<()> {
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
            env.get_null()
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
            env.get_null()
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
        &vec![
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
        &vec![
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
