use serde_derive::{Deserialize, Serialize};

/// File type.
#[derive(Clone, Serialize, Deserialize, Debug)]
pub struct FileType {
    pub name: String,
    pub extensions: Vec<String>,
}

impl FileType {
    pub fn new(name: &str, extensions: &[&str]) -> Self {
        FileType {
            name: name.into(),
            extensions: extensions.iter().map(|s| s.to_string()).collect(),
        }
    }
}
