use std::io;
use futures_io::{IoSlice};
use futures_io::AsyncWrite;
use futures_lite::AsyncWriteExt;


pub trait AsyncWriteVectoredExt: AsyncWrite + Unpin {
    /// Analogue to std::io::Write::write_all_vectored
    async fn write_all_vectored(&mut self, mut slices: &mut [IoSlice<'_>]) -> io::Result<()> {
        while !slices.is_empty() {
            match self.write_vectored(slices).await? {
                0 => return Err(io::Error::new(
                    io::ErrorKind::WriteZero,
                    "failed to write any elements from vectored buffer",
                )),
                written => {
                    IoSlice::advance_slices(&mut slices, written);
                }
            }
        }
        Ok(())
    }
}

impl<T: AsyncWrite + Unpin + ?Sized> AsyncWriteVectoredExt for T {}