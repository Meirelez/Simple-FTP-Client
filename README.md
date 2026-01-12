# Simple-FTP-Client

# FTP Client in C

A simple FTP client implemented in C using BSD sockets.

This project demonstrates how the FTP protocol works at a low level, including:
- Control and data connections
- Passive mode (PASV)
- Authentication
- Directory navigation
- Binary file download

## Features

- FTP URL parsing
- Anonymous and authenticated login
- Passive mode support
- Binary file download
- Directory traversal (CWD)
- File size query

## Usage

```bash
./ftpclient ftp://user:password@host/path/to/file
