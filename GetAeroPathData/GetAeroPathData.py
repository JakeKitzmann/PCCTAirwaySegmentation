import urllib.request, zipfile, os, glob
import sys

def GetAeroPathData(path = None):
    zip_path = os.path.join(path, "AeroPath.zip")
    os.makedirs(path, exist_ok=True)

    if os.path.exists(zip_path):
        os.remove(zip_path)
        print(f"Removed existing {zip_path}")

    url = "https://zenodo.org/records/10069289/files/AeroPath.zip?download=1"
    print('retrieving')
    urllib.request.urlretrieve(url, zip_path)

    with zipfile.ZipFile(zip_path) as z:
        print('extracting')
        z.extractall(path)

    for p in sorted(glob.glob(os.path.join(path, "**/*.nii.gz"), recursive=True))[:10]:
        print(p) 

if __name__ == "__main__":
    GetAeroPathData(sys.argv[1])    
    
# python GetAeroPathData.py /raid0/homes/jkitzmann/Research/PCCTAirwaySegmentation/  
