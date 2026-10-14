use clap::Parser;
use pe_save_editor::cli::{run_cli_command, Cli, Command};
use pe_save_editor::gui::run_gui;

fn main() -> anyhow::Result<()> {
    let cli = Cli::parse();
    match cli.command {
        None => run_gui(cli.path),
        Some(Command::Gui { path }) => run_gui(path.or(cli.path)),
        Some(other) => run_cli_command(other),
    }
}
