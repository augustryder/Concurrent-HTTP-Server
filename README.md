# Concurrent HTTP Server

A simple multithreaded HTTP server built in C using POSIX sockets and pthreads.

## Features

* **Socket Programming:** Implements TCP/IP communication using Berkeley sockets (POSIX).
* **Multithreading:** Implements concurrent request handling using the pthreads library.
* **Thread Pool:** Uses a custom thread pool with a concurrent queue for efficient thread management.
* **HTTP/1.x Support:** Handles GET requests with proper HTTP response headers.
* **Static File Serving:** Serves static HTML and JPEG content.

## Example

The server listens on port 8000 and serves:
* `/` - HTML page
* `/image` - JPEG image
* Other routes return 404

---

## Building and Running

### Prerequisites

* **C compiler**
* **pthread library**

### Compilation

```bash
```bash
make
```
```

### Running the Server

```bash
./server
```

The server will start listening on port `8000` on all interfaces.
You should probably not run this on a public WiFi network...

### Testing

Use any of the following methods to test:

**Using a web browser:**
```
http://localhost:8000/
```

**Using the included Python client:**
```bash
python3 client.py /
python3 client.py /image
```

**Using curl:**
```bash
curl http://localhost:8000/
```

**Load testing with Apache Bench:**
```bash
ab -n 1000 -c 50 http://localhost:8000/
```
