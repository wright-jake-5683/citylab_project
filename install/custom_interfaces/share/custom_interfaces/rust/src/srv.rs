

#[cfg(feature = "serde")]
use serde::{Deserialize, Serialize};

#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub struct GetDirection_Request {
    pub laser_data: sensor_msgs::msg::LaserScan,
}



impl Default for GetDirection_Request {
  fn default() -> Self {
    <Self as rosidl_runtime_rs::Message>::from_rmw_message(crate::srv::rmw::GetDirection_Request::default())
  }
}

impl rosidl_runtime_rs::Message for GetDirection_Request {
  type RmwMsg = crate::srv::rmw::GetDirection_Request;

  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> {
    match msg_cow {
      std::borrow::Cow::Owned(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
        laser_data: sensor_msgs::msg::LaserScan::into_rmw_message(std::borrow::Cow::Owned(msg.laser_data)).into_owned(),
      }),
      std::borrow::Cow::Borrowed(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
        laser_data: sensor_msgs::msg::LaserScan::into_rmw_message(std::borrow::Cow::Borrowed(&msg.laser_data)).into_owned(),
      })
    }
  }

  fn from_rmw_message(msg: Self::RmwMsg) -> Self {
    Self {
      laser_data: sensor_msgs::msg::LaserScan::from_rmw_message(msg.laser_data),
    }
  }
}


#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub struct GetDirection_Response {
    pub direction: std::string::String,
}



impl Default for GetDirection_Response {
  fn default() -> Self {
    <Self as rosidl_runtime_rs::Message>::from_rmw_message(crate::srv::rmw::GetDirection_Response::default())
  }
}

impl rosidl_runtime_rs::Message for GetDirection_Response {
  type RmwMsg = crate::srv::rmw::GetDirection_Response;

  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> {
    match msg_cow {
      std::borrow::Cow::Owned(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
        direction: msg.direction.as_str().into(),
      }),
      std::borrow::Cow::Borrowed(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
        direction: msg.direction.as_str().into(),
      })
    }
  }

  fn from_rmw_message(msg: Self::RmwMsg) -> Self {
    Self {
      direction: msg.direction.to_string(),
    }
  }
}






#[link(name = "custom_interfaces__rosidl_typesupport_c")]
extern "C" {
    fn rosidl_typesupport_c__get_service_type_support_handle__custom_interfaces__srv__GetDirection() -> *const std::os::raw::c_void;
}

// Corresponds to custom_interfaces__srv__GetDirection
pub struct GetDirection;

impl rosidl_runtime_rs::Service for GetDirection {
  type Request = crate::srv::GetDirection_Request;
  type Response = crate::srv::GetDirection_Response;

  fn get_type_support() -> *const std::os::raw::c_void {
    // SAFETY: No preconditions for this function.
    unsafe { rosidl_typesupport_c__get_service_type_support_handle__custom_interfaces__srv__GetDirection() }
  }
}



pub mod rmw {
#[cfg(feature = "serde")]
use serde::{Deserialize, Serialize};

#[link(name = "custom_interfaces__rosidl_typesupport_c")]
extern "C" {
    fn rosidl_typesupport_c__get_message_type_support_handle__custom_interfaces__srv__GetDirection_Request() -> *const std::os::raw::c_void;
}

#[link(name = "custom_interfaces__rosidl_generator_c")]
extern "C" {
    fn custom_interfaces__srv__GetDirection_Request__init(msg: *mut GetDirection_Request) -> bool;
    fn custom_interfaces__srv__GetDirection_Request__Sequence__init(seq: *mut rosidl_runtime_rs::Sequence<GetDirection_Request>, size: usize) -> bool;
    fn custom_interfaces__srv__GetDirection_Request__Sequence__fini(seq: *mut rosidl_runtime_rs::Sequence<GetDirection_Request>);
    fn custom_interfaces__srv__GetDirection_Request__Sequence__copy(in_seq: &rosidl_runtime_rs::Sequence<GetDirection_Request>, out_seq: *mut rosidl_runtime_rs::Sequence<GetDirection_Request>) -> bool;
}

// Corresponds to custom_interfaces__srv__GetDirection_Request
#[repr(C)]
#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub struct GetDirection_Request {
    pub laser_data: sensor_msgs::msg::rmw::LaserScan,
}



impl Default for GetDirection_Request {
  fn default() -> Self {
    unsafe {
      let mut msg = std::mem::zeroed();
      if !custom_interfaces__srv__GetDirection_Request__init(&mut msg as *mut _) {
        panic!("Call to custom_interfaces__srv__GetDirection_Request__init() failed");
      }
      msg
    }
  }
}

impl rosidl_runtime_rs::SequenceAlloc for GetDirection_Request {
  fn sequence_init(seq: &mut rosidl_runtime_rs::Sequence<Self>, size: usize) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { custom_interfaces__srv__GetDirection_Request__Sequence__init(seq as *mut _, size) }
  }
  fn sequence_fini(seq: &mut rosidl_runtime_rs::Sequence<Self>) {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { custom_interfaces__srv__GetDirection_Request__Sequence__fini(seq as *mut _) }
  }
  fn sequence_copy(in_seq: &rosidl_runtime_rs::Sequence<Self>, out_seq: &mut rosidl_runtime_rs::Sequence<Self>) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { custom_interfaces__srv__GetDirection_Request__Sequence__copy(in_seq, out_seq as *mut _) }
  }
}

impl rosidl_runtime_rs::Message for GetDirection_Request {
  type RmwMsg = Self;
  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> { msg_cow }
  fn from_rmw_message(msg: Self::RmwMsg) -> Self { msg }
}

impl rosidl_runtime_rs::RmwMessage for GetDirection_Request where Self: Sized {
  const TYPE_NAME: &'static str = "custom_interfaces/srv/GetDirection_Request";
  fn get_type_support() -> *const std::os::raw::c_void {
    // SAFETY: No preconditions for this function.
    unsafe { rosidl_typesupport_c__get_message_type_support_handle__custom_interfaces__srv__GetDirection_Request() }
  }
}


#[link(name = "custom_interfaces__rosidl_typesupport_c")]
extern "C" {
    fn rosidl_typesupport_c__get_message_type_support_handle__custom_interfaces__srv__GetDirection_Response() -> *const std::os::raw::c_void;
}

#[link(name = "custom_interfaces__rosidl_generator_c")]
extern "C" {
    fn custom_interfaces__srv__GetDirection_Response__init(msg: *mut GetDirection_Response) -> bool;
    fn custom_interfaces__srv__GetDirection_Response__Sequence__init(seq: *mut rosidl_runtime_rs::Sequence<GetDirection_Response>, size: usize) -> bool;
    fn custom_interfaces__srv__GetDirection_Response__Sequence__fini(seq: *mut rosidl_runtime_rs::Sequence<GetDirection_Response>);
    fn custom_interfaces__srv__GetDirection_Response__Sequence__copy(in_seq: &rosidl_runtime_rs::Sequence<GetDirection_Response>, out_seq: *mut rosidl_runtime_rs::Sequence<GetDirection_Response>) -> bool;
}

// Corresponds to custom_interfaces__srv__GetDirection_Response
#[repr(C)]
#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub struct GetDirection_Response {
    pub direction: rosidl_runtime_rs::String,
}



impl Default for GetDirection_Response {
  fn default() -> Self {
    unsafe {
      let mut msg = std::mem::zeroed();
      if !custom_interfaces__srv__GetDirection_Response__init(&mut msg as *mut _) {
        panic!("Call to custom_interfaces__srv__GetDirection_Response__init() failed");
      }
      msg
    }
  }
}

impl rosidl_runtime_rs::SequenceAlloc for GetDirection_Response {
  fn sequence_init(seq: &mut rosidl_runtime_rs::Sequence<Self>, size: usize) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { custom_interfaces__srv__GetDirection_Response__Sequence__init(seq as *mut _, size) }
  }
  fn sequence_fini(seq: &mut rosidl_runtime_rs::Sequence<Self>) {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { custom_interfaces__srv__GetDirection_Response__Sequence__fini(seq as *mut _) }
  }
  fn sequence_copy(in_seq: &rosidl_runtime_rs::Sequence<Self>, out_seq: &mut rosidl_runtime_rs::Sequence<Self>) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { custom_interfaces__srv__GetDirection_Response__Sequence__copy(in_seq, out_seq as *mut _) }
  }
}

impl rosidl_runtime_rs::Message for GetDirection_Response {
  type RmwMsg = Self;
  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> { msg_cow }
  fn from_rmw_message(msg: Self::RmwMsg) -> Self { msg }
}

impl rosidl_runtime_rs::RmwMessage for GetDirection_Response where Self: Sized {
  const TYPE_NAME: &'static str = "custom_interfaces/srv/GetDirection_Response";
  fn get_type_support() -> *const std::os::raw::c_void {
    // SAFETY: No preconditions for this function.
    unsafe { rosidl_typesupport_c__get_message_type_support_handle__custom_interfaces__srv__GetDirection_Response() }
  }
}






  #[link(name = "custom_interfaces__rosidl_typesupport_c")]
  extern "C" {
      fn rosidl_typesupport_c__get_service_type_support_handle__custom_interfaces__srv__GetDirection() -> *const std::os::raw::c_void;
  }

  // Corresponds to custom_interfaces__srv__GetDirection
  pub struct GetDirection;

  impl rosidl_runtime_rs::Service for GetDirection {
    type Request = crate::srv::rmw::GetDirection_Request;
    type Response = crate::srv::rmw::GetDirection_Response;

    fn get_type_support() -> *const std::os::raw::c_void {
      // SAFETY: No preconditions for this function.
      unsafe { rosidl_typesupport_c__get_service_type_support_handle__custom_interfaces__srv__GetDirection() }
    }
  }



}  // mod rmw
