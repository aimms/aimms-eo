import argparse
import os
import shutil
import stat

import subprocess
import urllib.request
import shutil

def download_file(url, dest):
    try:
        urllib.request.urlretrieve(url, dest)
    except urllib.error.HTTPError as e:
        raise Exception(f"HTTP Error: {e.code} - {e.reason}")
    except urllib.error.URLError as e:
        raise Exception(f"URL Error: {e.reason}")
    except Exception as e:
        raise Exception(f"Unexpected Error: {str(e)}")

def make_executable(path):
    st = os.stat(path)
    os.chmod(path, st.st_mode | stat.S_IEXEC)

def main():
    parser = argparse.ArgumentParser(description='Install AIMMS.')
    parser.add_argument('--version', required=True, help='Complete version of AIMMS in the format major.minor')
    args = parser.parse_args()
    
    # check if version is valid should be 1.1.1.1 major.minor.patch.revision
    if len(args.version.split('.')) != 4:
        print('Invalid version number. Please provide a version number in the format major.minor.patch.revision')
        return
    
    # should have 4 parts version number
    version_parts = args.version.split('.')
    
    for part in version_parts:
        part = part.lstrip('0')
        if not part.isdigit():
            print('Invalid version number. Please provide a version number in the format major.minor.patch.revision')
            return

    AIMMS_VERSION_MAJOR = version_parts[0].lstrip('0')
    AIMMS_VERSION_MINOR = version_parts[1].lstrip('0')
    AIMMS_VERSION_PATCH = version_parts[2].lstrip('0')
    AIMMS_VERSION_REVISION = version_parts[3].lstrip('0')
    
    installer_name_cap = f"Aimms-{AIMMS_VERSION_MAJOR}.{AIMMS_VERSION_MINOR}.{AIMMS_VERSION_PATCH}.{AIMMS_VERSION_REVISION}-installer.run"
    installer_name = f"aimms-{AIMMS_VERSION_MAJOR}.{AIMMS_VERSION_MINOR}.{AIMMS_VERSION_PATCH}.{AIMMS_VERSION_REVISION}-installer.run"
    installer_url = f"https://download.aimms.com/aimms/download/data/{AIMMS_VERSION_MAJOR}.{AIMMS_VERSION_MINOR}/{AIMMS_VERSION_PATCH}.{AIMMS_VERSION_REVISION}/"
    installer_path = os.path.join(os.getcwd(), installer_name)

    # Download the installer
    try:
        download_file(installer_url + installer_name, installer_path)
    except Exception as e:
        download_file(installer_url + installer_name_cap, installer_path)

    # Make the installer executable
    make_executable(installer_path)

    # Run the installer
    subprocess.run([installer_path, '--target', '/usr/local/Aimms', '--noexec'], check=True)

    # Clean up
    shutil.rmtree('/usr/local/Aimms/WebUIDev', ignore_errors=True)
    os.remove(installer_path)

if __name__ == "__main__":
    main()