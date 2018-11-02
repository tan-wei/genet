use inflector::cases::titlecase;

// From go-lint source code
// https://github.com/golang/lint
static WORDLIST: &[&str] = &[
    "ACL", "API", "ASCII", "CPU", "CSS", "DNS", "EOF", "GUID", "HTML", "HTTP", "HTTPS", "ID", "IP",
    "JSON", "LHS", "QPS", "RAM", "RHS", "RPC", "SLA", "SMTP", "SQL", "SSH", "TCP", "TLS", "TTL",
    "UDP", "UI", "UID", "UUID", "URI", "URL", "UTF8", "VM", "XML", "XMPP", "XSRF", "XSS",
];

pub fn to_title_case(s: &str) -> String {
    titlecase::to_title_case(s)
        .split_whitespace()
        .map(|w| {
            if WORDLIST.iter().any(|i| w.eq_ignore_ascii_case(i)) {
                w.to_uppercase()
            } else {
                w.to_string()
            }
        })
        .fold(String::new(), |acc, x| acc + &x + " ")
        .trim()
        .to_string()
}
