# voxl-rangefinder-service

A lightweight service for interfacing rangefinder sensors on the VOXL2 platform.

---

## 🧩 Dependencies

- [`voxl-docker`](https://docs.modalai.com/voxl-docker/)
- `voxl-cross:v4.4`
- `adb shell`

---

## 🏗️ Building the Project

### 1. Start the VOXL cross-compilation Docker image

From the **root of the project**, launch the VOXL cross-compilation environment:

```bash
sudo voxl-docker -i voxl-cross:v4.4
```

*(Replace the tag with your version if needed.)*

---

### 2. Build the Project

Inside the Docker container, run:

```bash
./build.sh qrb5165 <release>
```

Replace `<release>` with your desired build type (e.g., `debug`, `release`).

---

### 3. Create a `.deb` Package

Still inside the Docker container, package the build for deployment:

```bash
./make_package.sh qrb5165
```

---

### 4. Deploy to the VOXL Device

Exit the Docker container and deploy the package via SSH:

```bash
./deploy_to_voxl.sh ssh <voxl-ip>
```

Replace `<voxl-ip>` with your device’s IP address.

---

## ✅ Result

The package should now be installed and running on your VOXL.
You can verify using:

```bash
adb shell systemctl status voxl-rangefinder-service
```