import os
import shutil

# Build file types to delete
build_files = [
    "Makefile",
    ".sln",
    ".vcxproj",
    ".vcxproj.filters",
    ".user",
    ".log",
    ".profile.json",
]

# List of directories to clean
build_dirs = [
    "bin",
]

def delete_files_and_dirs(root_dir):
    # Traverse the directory and delete the specified build files
    for dirpath, dirnames, filenames in os.walk(root_dir, topdown=False):
        for filename in filenames:
            if any(filename.endswith(ext) for ext in build_files):
                file_path = os.path.join(dirpath, filename)
                try:
                    print(f"Deleting file: {file_path}")
                    os.remove(file_path)
                except Exception as e:
                    print(f"Error deleting file {file_path}: {e}")
        
        for dirname in dirnames:
            if dirname in build_dirs:
                dir_path = os.path.join(dirpath, dirname)
                try:
                    print(f"Deleting directory: {dir_path}")
                    shutil.rmtree(dir_path)
                except Exception as e:
                    print(f"Error deleting directory {dir_path}: {e}")

def main():
    root_directory = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    print(f"Starting cleanup in: {root_directory}")
    delete_files_and_dirs(root_directory)
    print("Cleanup completed.")

if __name__ == "__main__":
    main()