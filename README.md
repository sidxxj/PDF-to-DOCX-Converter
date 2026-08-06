# Offline PDF to DOCX & DOCX to PDF Converter

A lightweight, blazing-fast desktop utility designed to batch convert **PDF to DOCX** (Word) and **DOCX to PDF** locally on your machine. 

Unlike web-based converters, this application processes everything offline, ensuring absolute data privacy for sensitive documents. Built entirely in C++ using the Qt 6 framework, it leverages multithreading to keep the UI perfectly responsive even during heavy batch conversions.

## Core Features
* **Smart Bidirectional Conversion:** Automatically detects the file type. Drop a PDF, it converts to DOCX. Drop a Word document, it converts to PDF.
* **Batch Processing:** Drag and drop multiple files to convert them simultaneously.
* **100% Offline & Private:** No cloud uploads, no file size limits, and no internet connection required.
* **Multithreaded Engine:** Heavy document processing is offloaded to background threads.
* **Robust Error Handling:** 
  * Automatically detects and rejects password-encrypted PDFs.
  * Validates file magic numbers to prevent spoofed extension crashes.
  * Checks for OS-level read/write file locks before attempting conversion.
  * Dynamic processing timeouts based on file size (Megabytes).

## How it Works
The application acts as a high-performance C++ wrapper around the LibreOffice headless command-line engine. It safely manages background processes, file streams, and error streams, converting documents identically to native software exports.

## Installation & Requirements
1. **Operating System:** Windows or Linux.
2. **Dependencies:** [LibreOffice](https://www.libreoffice.org/) must be installed on your system. 
3. **Environment:** Ensure the LibreOffice `program` folder is added to your system's `PATH` variable.

## Usage
1. Launch the executable.
2. Drag and drop up to 10 `.pdf` or `.docx` files directly into the application window.
3. Click **Convert Files**.
4. The converted files will be automatically generated and saved to your system's default `Downloads` directory.

## Author & License
Developed by Mohammad Sirajuddin Mohammad Khalid.

This project is open-source and free to use. It is distributed under the **GNU GPLv3 License**. See the `LICENSE` file for more details.