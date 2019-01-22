use inflector::cases::titlecase;

// From go-lint source code
// https://github.com/golang/lint
static WORDLIST: &[&str] = &[
    "ACL", "API", "ASCII", "CPU", "CSS", "DNS", "EOF", "GUID", "HTML", "HTTP", "HTTPS", "ID", "IP",
    "JSON", "LHS", "QPS", "RAM", "RHS", "RPC", "SLA", "SMTP", "SQL", "SSH", "TCP", "TLS", "TTL",
    "UDP", "UI", "UID", "UUID", "URI", "URL", "UTF8", "VM", "XML", "XMPP", "XSRF", "XSS", "ARP",
    "IPv4", "IPv6",
];

pub fn to_title_case(s: &str) -> String {
    titlecase::to_title_case(s)
        .split_whitespace()
        .map(|w| {
            WORDLIST
                .iter()
                .find(|&i| w.eq_ignore_ascii_case(i))
                .cloned()
                .unwrap_or(w)
        })
        .fold(String::new(), |acc, x| acc + x + " ")
        .trim()
        .to_string()
}
