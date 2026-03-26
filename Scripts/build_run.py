import os
import subprocess
import argparse
import sys

from clean_build_files import delete_files_and_dirs

from typing import Literal

VS_DEV_CMD = r'"C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat"'
EXECUTABLE_PATH = "bin/LumoraApp"
SOLUTION_CANDIDATES = ("Lumora.slnx", "Lumora.sln")


def get_solution_file(project_dir: str) -> str | None:
    """Return the first existing solution file from known candidates."""
    for solution_file in SOLUTION_CANDIDATES:
        if os.path.isfile(os.path.join(project_dir, solution_file)):
            return solution_file
    return None


def get_sanitized_env() -> dict[str, str] | None:
    """On Windows, remove case-duplicate env vars that can break spawned tools."""
    if os.name != "nt":
        return None

    sanitized_env: dict[str, str] = {}
    seen_keys: set[str] = set()
    for key, value in os.environ.items():
        normalized_key = key.lower()
        if normalized_key in seen_keys:
            continue
        seen_keys.add(normalized_key)
        sanitized_env[key] = value
    return sanitized_env

def run_command(command: str, cwd: str = None) -> bool:
    """Run a shell command and print its output live."""
    try:
        print(f"Running command: {command}")
        env = get_sanitized_env()
        
        # Start the subprocess with Popen to capture stdout/stderr live
        with subprocess.Popen(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, cwd=cwd, env=env) as process:
            # Read the output line by line as it is produced
            for line in process.stdout:
                print(line, end='')  # Print standard output
            # Also print stderr (errors, if any)
            for line in process.stderr:
                print(line, end='', file=sys.stderr)  # Print error output

            # Wait for the process to finish and get the return code
            return_code = process.wait()

            if return_code != 0:
                print(f"Command failed with return code {return_code}")
                return False

            return True
            
    except Exception as e:
        print(f"Error running command: {e}")
        return False
    
def build_project(project_dir: str, generator: str) -> bool:
    """Generate build files using CMake."""
    cmake_command = f"premake5 {generator}"
    return run_command(cmake_command, cwd=project_dir)

def compile_project(project_dir: str, config: str, generator: str, system_type: Literal["Windows", "Linux"]) -> bool:
    """Compile the project using the appropriate build tool."""
    if system_type == "Linux" and generator.startswith("vs"):
        print("Visual Studio generators are not supported on Linux.")
        return False
    
    compile_command = ""
    if system_type == "Windows" and generator == "gmake2":
        compile_command = f"make CC=gcc config={config.lower()}"
    elif system_type == "Windows" and generator.startswith("vs"):
        solution_file = get_solution_file(project_dir)
        if not solution_file:
            print(f"No solution file found. Expected one of: {', '.join(SOLUTION_CANDIDATES)}")
            return False
        compile_command = f'cmd /c "call {VS_DEV_CMD} & msbuild {solution_file} /p:Configuration={config}"'
    elif system_type == "Linux" and generator == "gmake2":
        compile_command = f"make config={config.lower()}"

    if not compile_command:
        print("Unsupported generator or system type for compilation.")
        return False
    
    return run_command(compile_command, cwd=project_dir)

def run_project(project_dir: str, system_type: Literal["Windows", "Linux", "Unsupported"]) -> bool:
    """Run the compiled project."""
    exe_path = os.path.join(project_dir, EXECUTABLE_PATH + ('' if system_type == "Linux" else '.exe'))
    command = f"{exe_path}"
    return run_command(command, project_dir)

def display_details(args, project_dir: str, generator: str, config: str, system_type: Literal["Windows", "Linux", "Unsupported"]):
    print("Build and Run Configuration:")
    print(f"  Clean Build Files: {'Yes' if args.clean else 'No'} - Removes generated build artifacts (e.g., binaries, temporary files).")
    print(f"  Build Project: {'Yes' if args.build else 'No'} - Generates build files using the specified generator.")
    print(f"  Compile Project: {'Yes' if args.compile else 'No'} - Runs the build system to compile the code.")
    print(f"  Run Project: {'Yes' if args.run else 'No'} - Executes the compiled project.")
    print(f"  Project Directory: {project_dir}")
    print(f"  Build System Generator: {generator}")
    print(f"  Build Configuration: {config}")
    print(f"  Operating System: {system_type}\n")

def main():
    parser = argparse.ArgumentParser(description="Build and run the project")
    parser.add_argument("-x", "--clean", action="store_true", help="Clean build files before building", default=False)
    parser.add_argument("-b", "--build", action="store_true", help="Build the project", default=False)
    parser.add_argument("-c", "--compile", action="store_true", help="Compile the project", default=False)
    parser.add_argument("-r", "--run", action="store_true", help="Run the project", default=False)

    # Build system and build configuration
    parser.add_argument("-g", "--generator", type=str, choices=["gmake2", "vs2026"], default="gmake2", help="Specify the build system generator")
    parser.add_argument("-f", "--config", type=str, choices=["Debug", "Release", "Dist"], default="Release", help="Specify the build configuration")

    args = parser.parse_args()

    if not (args.clean or args.build or args.compile or args.run):
        args.clean = False
        args.build = True
        args.compile = True
        args.run = True

    project_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    system_type: Literal["Windows", "Linux", "Unsupported"] = "Windows" if os.name == "nt" else "Linux" if os.name == "posix" else "Unsupported"

    if system_type == "Unsupported":
        print("Operating system not supported by this script.")
        sys.exit(1)

    display_details(args, project_dir, args.generator, args.config, system_type)

    # Clean build files if requested
    if args.clean:
        print("Cleaning build files...")
        delete_files_and_dirs(project_dir)
        print("Clean completed.\n")

    # Build the project if requested
    if args.build:
        print("Building project...")
        status = build_project(project_dir, args.generator)
        if not status:
            print("Build failed.")
            sys.exit(1)
        print("Build completed.\n")

    # Compile the project if requested
    if args.compile:
        print("Compiling project...")
        status = compile_project(project_dir, args.config, args.generator, system_type)
        if not status:
            print("Compilation failed.")
            sys.exit(1)
        print("Compilation completed.\n")

    # Run the project if requested
    if args.run:
        print("Running project...\n")
        status = run_project(project_dir, system_type)

if __name__ == "__main__":
    main()
