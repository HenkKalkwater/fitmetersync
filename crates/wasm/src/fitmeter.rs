use js_sys::{Array, BigInt, Date, Number};
use wasm_bindgen::{JsCast, JsError, JsValue};
use wasm_bindgen::prelude::wasm_bindgen;
use wasm_bindgen::sys::JsNullable;
use web_sys::{ReadableStream, WritableStream};
use crate::{NullableNumberArray, NumberArray};
use crate::io::Port;

#[wasm_bindgen]
pub struct FitMeterSync {
    fms: fms_samu::FitMeterSync<Port>
}

#[wasm_bindgen]
pub struct FmsIdentity {
    #[wasm_bindgen(getter_with_clone)]
    pub user_id: BigInt,
    #[wasm_bindgen(getter_with_clone)]
    pub samu_id: BigInt,
    pub user_flags: u32
}

#[wasm_bindgen]
impl FmsIdentity {
    fn new(user_id: BigInt, samu_id: BigInt, user_flags: u32) -> Self {
        FmsIdentity {
            user_id,
            samu_id,
            user_flags
        }
    }
}


#[wasm_bindgen]
impl FitMeterSync {

    /// Creates a FitMeterSync from a readable and writable stream
    #[wasm_bindgen(constructor)]
    pub fn new(readable_stream: ReadableStream, writable_stream: WritableStream) -> Self {
        FitMeterSync {
            fms: fms_samu::FitMeterSync::new(
                Port::new(readable_stream, writable_stream)
            )
        }
    }

    /// Creates a FitMeter by opening a serial port
    /// Note: requires user interaction, because it uses WebSerial
    pub async fn open_serial() -> Result<Self, JsError> {
        Ok(
            FitMeterSync {
                fms: fms_samu::FitMeterSync::new(
                    Port::open_serial().await?
                )
            }
        )
    }

    pub async fn connect(&mut self) -> Result<(), JsError> {
        self.fms.connect().await?;
        Ok(())
    }

    pub async fn disconnect(&mut self) -> Result<(), JsError> {
        self.fms.disconnect().await?;
        Ok(())
    }

    pub async fn get_identity(&mut self) -> Result<FmsIdentity, JsValue> {
        let ident = self.fms.get_identity().await
            .map_err(|e| JsError::new(format!("Failed to get identity: {:?}", e).as_str()))?;

        let result = FmsIdentity::new(
            ident.user_id.into(),
            ident.fitmeter_id.into(),
            ident.flag.into()
        );

        Ok(result.into())
    }

    pub async fn get_time(&mut self) -> Result<Date, JsValue> {
        let time = self.fms.get_time().await
            .map_err(|e| JsError::new(format!("Failed to get time: {:?}", e).as_str()))?;

        let js_time = Date::new_with_year_month_day_hr_min(
            time.year(),
            time.month() as i32,
            time.day() as i32,
            time.hour() as i32,
            time.minute() as i32);

        Ok(js_time)
    }

    pub async fn get_altitude(&mut self) -> Result<NumberArray, JsValue> {
        let altitude = self.fms.get_altitude().await
            .map_err(|e| JsError::new(format!("Failed to get altitude: {:?}", e).as_str()))?;

        let iter = altitude.iter()
            .map(|i| Number::from(*i));

        Ok(Array::from_iter(iter).unchecked_into())
    }

    pub async fn get_activity_tag(&mut self) -> Result<NullableNumberArray, JsValue> {
        let activity_tag = self.fms.get_activity_tag().await
            .map_err(|e| JsError::new(format!("Failed to get activity tag: {:?}", e).as_str()))?;

        let iter = activity_tag.iter()
            .map(|tag| JsNullable::from_option(tag.map(Number::from)));

        Ok(Array::from_iter(iter).unchecked_into())
    }

    pub async fn get_mets(&mut self) -> Result<NullableNumberArray, JsValue> {
        let mets = self.fms.get_mets().await
            .map_err(|e| JsError::new(format!("Failed to get mets: {:?}", e).as_str()))?;

        let iter = mets.iter()
            .map(|met| JsNullable::from_option(met.map(Number::from)));

        Ok(Array::from_iter(iter).unchecked_into())
    }

    pub async fn get_kcals(&mut self) -> Result<NullableNumberArray, JsValue> {
        let kcals = self.fms.get_kcals().await
            .map_err(|e| JsError::new(format!("Failed to get kcals: {:?}", e).as_str()))?;

        let iter = kcals.iter()
            .map(|kcal| JsNullable::from_option(kcal.map(Number::from)));

        Ok(Array::from_iter(iter).unchecked_into())
    }

    pub async fn get_steps(&mut self) -> Result<NullableNumberArray, JsValue> {
        let steps = self.fms.get_steps().await
            .map_err(|e| JsError::new(format!("Failed to get steps: {:?}", e).as_str()))?;

        let iter = steps.iter()
            .map(|step| JsNullable::from_option(step.map(Number::from)));

        Ok(Array::from_iter(iter).unchecked_into())
    }

}