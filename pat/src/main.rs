// Print Address Table (PAT)
//
// Simple program to read import and export table from PE/COFF executables
// and Dynamic Linked Libraries (DLLs). Added bonus, it does print out a C++
// source code that can be used with Visual Studio 7+ to compile a proxy DLL
// for launching DLL redirection/side-loading attacks.
//
// Written by Nicolas Chabbey (keybase: e3prom)
use getopts::Options;
use goblin::{pe, Object};
use std::env;
use std::error::Error;
use std::fmt;
use std::fs::File;
use std::io::Read;
use std::path::Path;

// MyError Type
#[derive(Debug)]
struct MyError(String);

impl fmt::Display for MyError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "Error: {}", self.0)
    }
}
impl Error for MyError {}

// Config structure
struct Config {
    action: i32,
    filename: String,
    dll: String,
}

impl Config {
    fn new(action: i32, filename: String, dll: String) -> Config {
        Config {
            action,
            filename,
            dll,
        }
    }
}

// main function
fn main() {
    let args: Vec<String> = env::args().collect();

    match parse_cl_opts(&args) {
        Err(e) => {
            eprintln!("{}", e);
            std::process::exit(1);
        }
        Ok(c) => match run(c) {
            Err(e) => {
                eprintln!("{}", e);
                std::process::exit(1);
            }
            Ok(_) => {
                std::process::exit(0);
            }
        },
    }
}

// print_usage function
fn print_usage(program: &str, opts: Options) {
    let actions = format!(
        "Actions:
      1 = Print Import Address Table (IAT)
      2 = Print Export Address Table (EAT)
      3 = Print DLL Proxy Source (C++ Visual Studio)"
    );
    let brief = format!(
        "Usage: {} -A <1|2|3> [options] -f <PE/COFF executable or DLL>\n\n{}",
        program, actions
    );
    print!("{}", opts.usage(&brief));
}

// parse_cl_opts function
fn parse_cl_opts(args: &[String]) -> Result<Config, Box<dyn Error>> {
    let program = args[0].clone();
    let mut opts = Options::new();

    opts.optopt("A", "action", "action to perform (see Actions)", "ACTION");
    opts.optopt(
        "f",
        "file",
        "file path to a PE/COFF executable or DLL",
        "FILE",
    );
    opts.optopt("d", "dll", "DLL file to proxy function calls to", "FILE");
    opts.optflag("h", "help", "display help information");

    let matches = match opts.parse(&args[1..]) {
        Ok(m) => m,
        Err(f) => return Result::Err(Box::new(MyError(f.to_string().into()))),
    };

    if matches.opt_present("help") || args[1..].is_empty() {
        print_usage(&program, opts);
        std::process::exit(1);
    }

    // -A | --action
    let action = matches.opt_str("action");
    let action = match action {
        Some(v) => v.parse::<i32>().unwrap(),
        None => return Result::Err(Box::new(MyError("No action specified (-A).".into()))),
    };

    // -f | --filename
    let filename = matches.opt_str("file");
    let filename = match filename {
        Some(v) => v,
        None => {
            return Result::Err(Box::new(MyError(
                "No PE or DLL filename specified (-f).".into(),
            )));
        }
    };

    // -d | --dll
    let dll = matches.opt_str("dll");
    let dll = match dll {
        Some(v) => v,
        None => {
            if action == 3 {
                return Result::Err(Box::new(MyError("No DLL filename specified (-d).".into())));
            } else {
                std::string::String::new()
            }
        }
    };

    Ok(Config::new(action, filename, dll))
}

// run function
fn run(cfg: Config) -> Result<(), Box<dyn Error>> {
    let path = Path::new(cfg.filename.as_str());
    let mut fd = File::open(path)?;
    let mut buffer = Vec::new();

    fd.read_to_end(&mut buffer)?;
    match Object::parse(&buffer)? {
        Object::PE(pe) => {
            //println!("debug: {:#?}", &pe);
            match cfg.action {
                1 => show_pe_coff_import(pe),
                2 => show_pe_coff_export(pe),
                3 => show_vs_dll_sideload(cfg, pe),
                _ => return Result::Err(Box::new(MyError("Unknown action specified.".into()))),
            }
        }
        _ => {
            return Result::Err(Box::new(MyError(
                "Please specify a PE/COFF executable or a DLL.".into(),
            )));
        }
    }
}

// show_pe_coff_import function
fn show_pe_coff_import(pe: goblin::pe::PE) -> Result<(), Box<dyn Error>> {
    let header = "library name: ";
    let entry = " ".repeat(2);

    let import = match pe.import_data {
        Some(i) => i,
        None => {
            return Result::Err(Box::new(MyError(
                "Executable or DLL has no import table.".into(),
            )));
        }
    };

    for e in import.import_data {
        let hline = "-".repeat(header.len() + e.name.len());
        println!("{}{}\n{}", header, e.name, hline);

        println!("imported functions:");
        for v in e.import_lookup_table {
            for s in v.iter() {
                if let pe::import::SyntheticImportLookupTableEntry::HintNameTableRVA((r, h)) = s {
                    println!(" - {}()", h.name);
                    println!("{} * RVA: {}", entry, r);
                    println!("{} * Ordinal: {}", entry, h.hint);
                }
            }
        }
        println!("\n");
    }

    Ok(())
}

// show_pe_coff_export function
fn show_pe_coff_export(pe: goblin::pe::PE) -> Result<(), Box<dyn Error>> {
    let header = "library name: ";
    let entry = " ".repeat(2);

    let name = match pe.name {
        Some(n) => n,
        None => {
            return Result::Err(Box::new(MyError(
                "Executable or DLL has no export table.".into(),
            )));
        }
    };

    let hline = "-".repeat(header.len() + name.len());
    println!("{}{}\n{}", header, name, hline);

    println!("exported function:");
    for e in pe.exports {
        if let Some(name) = e.name {
            println!(" - {}()\n   * RVA: {}", name, e.rva);
            println!("{} * Offset: {}", entry, e.offset);
        }
    }

    Ok(())
}

// show_vs_dll_sideload
fn show_vs_dll_sideload(cfg: Config, pe: goblin::pe::PE) -> Result<(), Box<dyn Error>> {
    // sideload source header
    let header = format!(
        "#include \"stdafx.h\"

HINSTANCE hDll = LoadLibraryA(\"{}\");\n\n",
        cfg.dll
    );

    print!("{}", &header);

    // format string variables
    let proxy: Vec<&str> = cfg.dll.split(".").collect();
    let mut ord: u16 = 1;

    // sideload source body
    for e in pe.exports {
        if let Some(func) = e.name {
            println!(
                "#pragma comment(linker, \"/export:{}={}.{},@{}\")",
                func, proxy[0], func, ord
            );
            ord = ord + 1;
        }
    }

    Ok(())
}
