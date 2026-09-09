use std::io;
use futures_io::{IoSlice};
use futures_io::AsyncWrite;
use futures_lite::AsyncWriteExt;


pub trait AsyncWriteVectoredExt: AsyncWrite + Unpin {
    /// Writes an entire sequence of vectored buffers to the stream asynchronously.
    async fn write_all_vectored(&mut self, mut slices: &mut [IoSlice<'_>]) -> io::Result<()> {
        while !slices.is_empty() {
            // 1. Perform a single runtime-agnostic vectored write
            let written = self.write_vectored(slices).await?;

            if written == 0 {
                return Err(io::Error::new(
                    io::ErrorKind::WriteZero,
                    "failed to write any elements from vectored buffer",
                ));
            }

            // 2. Loop re-binding trick to avoid the nested &mut borrow checker trap
            let mut rest = slices;
            IoSlice::advance_slices(&mut rest, written);
            slices = rest;
        }
        Ok(())
    }
}

impl<T: AsyncWrite + Unpin + ?Sized> AsyncWriteVectoredExt for T {}