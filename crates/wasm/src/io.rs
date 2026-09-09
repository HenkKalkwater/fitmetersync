use std::io::{IoSlice, IoSliceMut};
use std::pin::{pin, Pin};
use std::task::{Context, Poll};
use futures::{AsyncRead, AsyncWrite};
use wasm_bindgen::{JsError};
use wasm_streams::{ReadableStream, WritableStream};
use web_sys::{SerialOptions, ReadableStream as JsReadableStream, WritableStream as JsWritableStream};

pub struct Port {
    read_stream: Box<dyn AsyncRead + Unpin>,
    write_stream: Box<dyn AsyncWrite + Unpin>
}

impl Port {
    pub fn new(read_stream: JsReadableStream, write_stream: JsWritableStream) -> Self {
        Port {
            read_stream: Box::new(ReadableStream::from_raw(read_stream).into_async_read()),
            write_stream: Box::new(WritableStream::from_raw(write_stream).into_async_write())
        }
    }

    pub async fn open_serial() -> Result<Self, JsError> {
        let window = web_sys::window().ok_or(JsError::new("No window"))?;
        let navigator = window.navigator();
        let serial = navigator.serial();
        let port = serial.request_port().await
            .map_err(|e| JsError::new(format!("No port: {e:?}").as_str()))?;

        let port_options = SerialOptions::new(115_200);

        port.open(&port_options).await
            .map_err(|e| JsError::new(format!("Failed to open port: {e:?}").as_str()))?;

        Ok(Port::new(port.readable(), port.writable()))
    }
}

impl AsyncRead for Port {
    fn poll_read(self: Pin<&mut Self>, cx: &mut Context<'_>, buf: &mut [u8]) -> Poll<std::io::Result<usize>> {
        let this = self.get_mut();
        pin!(&mut *this.read_stream).poll_read(cx, buf)
    }

    fn poll_read_vectored(self: Pin<&mut Self>, cx: &mut Context<'_>, bufs: &mut [IoSliceMut<'_>]) -> Poll<std::io::Result<usize>> {
        let this = self.get_mut();
        pin!(&mut *this.read_stream).poll_read_vectored(cx, bufs)
    }
}

impl AsyncWrite for Port {
    fn poll_write(self: Pin<&mut Self>, cx: &mut Context<'_>, buf: &[u8]) -> Poll<std::io::Result<usize>> {
        let this = self.get_mut();
        pin!(&mut *this.write_stream).poll_write(cx, buf)
    }

    fn poll_write_vectored(self: Pin<&mut Self>, cx: &mut Context<'_>, bufs: &[IoSlice<'_>]) -> Poll<std::io::Result<usize>> {
        let this = self.get_mut();
        pin!(&mut *this.write_stream).poll_write_vectored(cx, bufs)
    }

    fn poll_flush(self: Pin<&mut Self>, cx: &mut Context<'_>) -> Poll<std::io::Result<()>> {
        let this = self.get_mut();
        pin!(&mut *this.write_stream).poll_flush(cx)
    }

    fn poll_close(self: Pin<&mut Self>, cx: &mut Context<'_>) -> Poll<std::io::Result<()>> {
        let this = self.get_mut();
        pin!(&mut *this.write_stream).poll_close(cx)
    }
}

