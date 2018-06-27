use genet_abi::token::Token;

#[macro_export]
macro_rules! token {
    () => {{
        use genet_abi::token::Token;
        Token::null()
    }};
    ($name:expr) => {{
        use genet_abi::token::Token;
        thread_local!(static TOKEN: Token = { Token::from($name) };);
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
        assert_eq!(token.to_string(), "eth");
        let token = token!("[eth]");
        assert_eq!(token.to_string(), "[eth]");
        let token = token!("eth");
        assert_eq!(token.to_string(), "eth");
        let token = token!();
        assert_eq!(token.to_string(), "");
        let token = token!("dd31817d-1501-4b2b-bcf6-d02e148d3ab9");
        assert_eq!(token.to_string(), "dd31817d-1501-4b2b-bcf6-d02e148d3ab9");
        assert_eq!(Token::from(1000).to_string(), "");
    }
}
