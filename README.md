# 🚀 Vulkan + GLFW C++ Game Engine (WIP)

Минималистичный C++ проект для изучения и разработки собственного **game engine**  
на базе **Vulkan** и **GLFW**.

> 🔧 Проект в активной разработке. Цель — чистая архитектура, высокая производительность и минимум лишних зависимостей.

---

## ✨ Возможности

- 🧱 CMake-based сборка
- 🔥 Vulkan renderer
- 🪟 GLFW window system
- ⚡ Чистый C++ (без heavy-фреймворков)
- 🧠 Основа для собственного engine

---

## 📁 Структура проекта
c++_Game_Engine/

│── CMakeLists.txt

│── main.cpp

│── GLFW/              # GLFW headers

│── build/             # build directory (НЕ коммитится)

│── README.md

│── .gitignore


---

## 🛠 Зависимости

### Общие
- **CMake ≥ 3.20**
- **C++17**
- **GLFW**
- **Vulkan SDK**

---

## 🍎 Сборка на macOS

### 1️⃣ Установи зависимости
bash
```
brew install cmake glfw
```

Установи Vulkan SDK с официального сайта:
https://vulkan.lunarg.com
Проверь:
```
vulkaninfo
```

2️⃣ Сборка проекта
```
git clone git@github.com:GOSPODINYON/ip_Vulkan-GLFW.git
cd ip_Vulkan-GLFW

mkdir build
cd build
cmake ..
cmake --build .
```

3️⃣ Запуск
```
./VulkanWindow
```

🐧 Сборка на Linux

1️⃣ Установи зависимости (Ubuntu / Debian)

```
sudo apt update
sudo apt install -y \
  cmake \
  g++ \
  libglfw3-dev \
  vulkan-sdk \
  vulkan-validationlayers-dev
```

2️⃣ Сборка

```
git clone https://github.com/GOSPODINYON/ip_Vulkan-GLFW.git
cd ip_Vulkan-GLFW

mkdir build
cd build
cmake ..
make
```

3️⃣ Запуск
```
./VulkanWindow
```

🪟 Сборка на Windows (MSVC)

1️⃣ Установи
    •    Visual Studio 2022
    •    ✔ Desktop development with C++
    •    CMake
    •    Vulkan SDK
    •    GLFW


2️⃣ Сборка
```
git clone https://github.com/GOSPODINYON/ip_Vulkan-GLFW.git
cd ip_Vulkan-GLFW

mkdir build
cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
```

3️⃣ Запуск
```
Release\VulkanWindow.exe
```
