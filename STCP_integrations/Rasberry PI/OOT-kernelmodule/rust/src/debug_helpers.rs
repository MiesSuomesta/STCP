
use core::fmt;
use core::fmt::Write as FmtWrite;
use alloc::string::String;

// Usage:
// stcp_dbg!("RX raw hex");   
// stcp_dbg!("RX ascii");   

/// ASCII-dumppi: ei-tulostettavat merkit → '.'
pub struct AsciiDump<'a>(pub &'a [u8]);

impl<'a> fmt::Display for AsciiDump<'a> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let bytes = self.0;

        for (i, b) in bytes.iter().enumerate() {
            let mut ch = *b as char;

            // sallitaan välilyönti sellaisenaan
            if *b != b' ' {
                if !ch.is_ascii_graphic() {
                    ch = '.';
                }
            }

            write!(f, "{}", ch)?;

            if i + 1 < bytes.len() {
                f.write_str(" ")?; // väli merkkien väliin
            }
        }

        Ok(())
    }
}

/// Hex-dumppi: "AA BB CC ..."
pub struct HexDump<'a>(pub &'a [u8]);

impl<'a> fmt::Display for HexDump<'a> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let bytes = self.0;

        for (i, b) in bytes.iter().enumerate() {
            write!(f, "{:02X}", b)?;

            if i + 1 < bytes.len() {
                f.write_str(" ")?; // väli tavujen väliin
            }
        }

        Ok(())
    }
}

/// Hex-dumppi → String
pub fn hex_dump_to_string(bytes: &[u8]) -> String {
    // arvio: "XX " * n -> 3 * n merkkiä, mutta ei haittaa jos menee vähän ohi
    let mut s = String::with_capacity(bytes.len().saturating_mul(3));

    for (i, b) in bytes.iter().enumerate() {
        let _ = write!(s, "{:02X}", b);
        if i + 1 < bytes.len() {
            let _ = s.write_str(" ");
        }
    }

    s
}

/// ASCII-dumppi → String
pub fn ascii_dump_to_string(bytes: &[u8]) -> String {
    let mut s = String::with_capacity(bytes.len().saturating_mul(2));

    for (i, b) in bytes.iter().enumerate() {
        let mut ch = *b as char;

        if *b != b' ' {
            if !ch.is_ascii_graphic() {
                ch = '.';
            }
        }

        let _ = write!(s, "{}", ch);
        if i + 1 < bytes.len() {
            let _ = s.write_str(" ");
        }
    }

    s
}
