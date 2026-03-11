// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from custom_interfaces:action/GoToPose.idl
// generated code does not contain a copyright notice

#ifndef CUSTOM_INTERFACES__ACTION__DETAIL__GO_TO_POSE__STRUCT_H_
#define CUSTOM_INTERFACES__ACTION__DETAIL__GO_TO_POSE__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Constants defined in the message

// Include directives for member types
// Member 'goal_pos'
#include "geometry_msgs/msg/detail/pose2_d__struct.h"

/// Struct defined in action/GoToPose in the package custom_interfaces.
typedef struct custom_interfaces__action__GoToPose_Goal
{
  geometry_msgs__msg__Pose2D goal_pos;
} custom_interfaces__action__GoToPose_Goal;

// Struct for a sequence of custom_interfaces__action__GoToPose_Goal.
typedef struct custom_interfaces__action__GoToPose_Goal__Sequence
{
  custom_interfaces__action__GoToPose_Goal * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} custom_interfaces__action__GoToPose_Goal__Sequence;


// Constants defined in the message

/// Struct defined in action/GoToPose in the package custom_interfaces.
typedef struct custom_interfaces__action__GoToPose_Result
{
  bool status;
} custom_interfaces__action__GoToPose_Result;

// Struct for a sequence of custom_interfaces__action__GoToPose_Result.
typedef struct custom_interfaces__action__GoToPose_Result__Sequence
{
  custom_interfaces__action__GoToPose_Result * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} custom_interfaces__action__GoToPose_Result__Sequence;


// Constants defined in the message

// Include directives for member types
// Member 'current_pos'
// already included above
// #include "geometry_msgs/msg/detail/pose2_d__struct.h"

/// Struct defined in action/GoToPose in the package custom_interfaces.
typedef struct custom_interfaces__action__GoToPose_Feedback
{
  geometry_msgs__msg__Pose2D current_pos;
} custom_interfaces__action__GoToPose_Feedback;

// Struct for a sequence of custom_interfaces__action__GoToPose_Feedback.
typedef struct custom_interfaces__action__GoToPose_Feedback__Sequence
{
  custom_interfaces__action__GoToPose_Feedback * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} custom_interfaces__action__GoToPose_Feedback__Sequence;


// Constants defined in the message

// Include directives for member types
// Member 'goal_id'
#include "unique_identifier_msgs/msg/detail/uuid__struct.h"
// Member 'goal'
#include "custom_interfaces/action/detail/go_to_pose__struct.h"

/// Struct defined in action/GoToPose in the package custom_interfaces.
typedef struct custom_interfaces__action__GoToPose_SendGoal_Request
{
  unique_identifier_msgs__msg__UUID goal_id;
  custom_interfaces__action__GoToPose_Goal goal;
} custom_interfaces__action__GoToPose_SendGoal_Request;

// Struct for a sequence of custom_interfaces__action__GoToPose_SendGoal_Request.
typedef struct custom_interfaces__action__GoToPose_SendGoal_Request__Sequence
{
  custom_interfaces__action__GoToPose_SendGoal_Request * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} custom_interfaces__action__GoToPose_SendGoal_Request__Sequence;


// Constants defined in the message

// Include directives for member types
// Member 'stamp'
#include "builtin_interfaces/msg/detail/time__struct.h"

/// Struct defined in action/GoToPose in the package custom_interfaces.
typedef struct custom_interfaces__action__GoToPose_SendGoal_Response
{
  bool accepted;
  builtin_interfaces__msg__Time stamp;
} custom_interfaces__action__GoToPose_SendGoal_Response;

// Struct for a sequence of custom_interfaces__action__GoToPose_SendGoal_Response.
typedef struct custom_interfaces__action__GoToPose_SendGoal_Response__Sequence
{
  custom_interfaces__action__GoToPose_SendGoal_Response * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} custom_interfaces__action__GoToPose_SendGoal_Response__Sequence;


// Constants defined in the message

// Include directives for member types
// Member 'goal_id'
// already included above
// #include "unique_identifier_msgs/msg/detail/uuid__struct.h"

/// Struct defined in action/GoToPose in the package custom_interfaces.
typedef struct custom_interfaces__action__GoToPose_GetResult_Request
{
  unique_identifier_msgs__msg__UUID goal_id;
} custom_interfaces__action__GoToPose_GetResult_Request;

// Struct for a sequence of custom_interfaces__action__GoToPose_GetResult_Request.
typedef struct custom_interfaces__action__GoToPose_GetResult_Request__Sequence
{
  custom_interfaces__action__GoToPose_GetResult_Request * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} custom_interfaces__action__GoToPose_GetResult_Request__Sequence;


// Constants defined in the message

// Include directives for member types
// Member 'result'
// already included above
// #include "custom_interfaces/action/detail/go_to_pose__struct.h"

/// Struct defined in action/GoToPose in the package custom_interfaces.
typedef struct custom_interfaces__action__GoToPose_GetResult_Response
{
  int8_t status;
  custom_interfaces__action__GoToPose_Result result;
} custom_interfaces__action__GoToPose_GetResult_Response;

// Struct for a sequence of custom_interfaces__action__GoToPose_GetResult_Response.
typedef struct custom_interfaces__action__GoToPose_GetResult_Response__Sequence
{
  custom_interfaces__action__GoToPose_GetResult_Response * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} custom_interfaces__action__GoToPose_GetResult_Response__Sequence;


// Constants defined in the message

// Include directives for member types
// Member 'goal_id'
// already included above
// #include "unique_identifier_msgs/msg/detail/uuid__struct.h"
// Member 'feedback'
// already included above
// #include "custom_interfaces/action/detail/go_to_pose__struct.h"

/// Struct defined in action/GoToPose in the package custom_interfaces.
typedef struct custom_interfaces__action__GoToPose_FeedbackMessage
{
  unique_identifier_msgs__msg__UUID goal_id;
  custom_interfaces__action__GoToPose_Feedback feedback;
} custom_interfaces__action__GoToPose_FeedbackMessage;

// Struct for a sequence of custom_interfaces__action__GoToPose_FeedbackMessage.
typedef struct custom_interfaces__action__GoToPose_FeedbackMessage__Sequence
{
  custom_interfaces__action__GoToPose_FeedbackMessage * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} custom_interfaces__action__GoToPose_FeedbackMessage__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // CUSTOM_INTERFACES__ACTION__DETAIL__GO_TO_POSE__STRUCT_H_
