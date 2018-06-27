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
    () => {{
        use genet_abi::token::Token;
        Token::null()
    }};
    ($name:expr) => {{
        use genet_abi::env;
        use genet_abi::token::Token;
        thread_local!(static TOKEN: Token = { env::token($name as &'static str) };);
        TOKEN.with(|&t| t)
    }};
}

#[cfg(test)]
mod tests {
    use genet_abi::token::Token;

    #[test]
    fn token() {
        pub use genet_abi::env;
        assert_eq!(token!(), Token::null());
        let token = token!("eth");
        assert_eq!(env::string(token), "eth");
        let token = token!("[eth]");
        assert_eq!(env::string(token), "[eth]");
        let token = token!("eth");
        assert_eq!(env::string(token), "eth");
        let token = token!();
        assert_eq!(env::string(token), "");
        let token = token!("dd31817d-1501-4b2b-bcf6-d02e148d3ab9");
        assert_eq!(env::string(token), "dd31817d-1501-4b2b-bcf6-d02e148d3ab9");
        assert_eq!(env::string(Token::new(1000)), "");
    }
}
