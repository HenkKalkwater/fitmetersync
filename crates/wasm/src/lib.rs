extern crate core;

pub mod utils;
pub mod fitmeter;
pub mod io;

use wasm_bindgen::prelude::*;

#[wasm_bindgen]
extern "C" {
    #[wasm_bindgen(typescript_type = "Array<Number|null>")]
    pub type NullableNumberArray;

    #[wasm_bindgen(typescript_type = "Array<Number>")]
    pub type NumberArray;
}

