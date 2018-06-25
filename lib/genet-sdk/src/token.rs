use genet_abi::env;
use genet_abi::token::Token;

pub fn get(id: &str) -> Token {
    env::token(id)
}

pub fn string(id: Token) -> String {
    env::string(id)
}

#[macro_export]
macro_rules! token {
    ($name:expr) => {{
        pub use genet_abi::env;
        pub use genet_abi::token::Token;
        thread_local!(static TOKEN: Token = { env::token($name as &'static str) };);
        TOKEN.with(|&t| t)
    }};
}

#[cfg(test)]
mod tests {
    #[test]
    fn token() {
        pub use genet_abi::env;
        assert_eq!(token!(""), 0);
        let token = token!("eth");
        assert_eq!(env::string(token), "eth");
        let token = token!("[eth]");
        assert_eq!(env::string(token), "[eth]");
        let token = token!("eth");
        assert_eq!(env::string(token), "eth");
        let token = token!("");
        assert_eq!(env::string(token), "");
        let token = token!("dd31817d-1501-4b2b-bcf6-d02e148d3ab9");
        assert_eq!(env::string(token), "dd31817d-1501-4b2b-bcf6-d02e148d3ab9");
        assert_eq!(env::string(1000), "");
    }
}
