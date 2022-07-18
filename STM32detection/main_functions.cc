/* Copyright 2019 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "main_functions.h"

#include "constants.h"
#include "output_handler.h"
#include "model_gestures.h"
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"
#include "printf.h"
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/usart.h> 
#include <gyro.h>
////#include <console.h>
const int numSamples = 39;
int samplesRead = 0;
// Globals, used for compatibility with Arduino-style sketches.
namespace {
tflite::ErrorReporter* error_reporter = nullptr;
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;
//int inference_count = 0;

// Create an area of memory to use for input, output, and intermediate arrays.
// Finding the minimum value for your model may require some trial and error.
constexpr int kTensorArenaSize = 4 * 1024;
uint8_t tensor_arena[kTensorArenaSize];
}  // namespace

const char* GESTURES[] = {
  "updown",
  "estacionario",
  "leftright"
};

static void my_usart_print_int(uint32_t usart, int32_t value)
{
	int8_t i;
	int8_t nr_digits = 0;
	char buffer[25];

	if (value < 0) {
		usart_send_blocking(usart, '-');
		value = value * -1;
	}

	if (value == 0) {
		usart_send_blocking(usart, '0');
	}

	while (value > 0) {
		buffer[nr_digits++] = "0123456789"[value % 10];
		value /= 10;
	}

	for (i = nr_digits-1; i >= 0; i--) {
		usart_send_blocking(usart, buffer[i]);
	}

	//usart_send_blocking(usart, '\t');
}

#define NUM_GESTURES (sizeof(GESTURES) / sizeof(GESTURES[0]))

// The name of this function is important for Arduino compatibility.
void setup() {
  // Set up logging. Google style is to avoid globals or statics because of
  // lifetime uncertainty, but since this has a trivial destructor it's okay.
  // NOLINTNEXTLINE(runtime-global-variables)
  static tflite::MicroErrorReporter micro_error_reporter;
  error_reporter = &micro_error_reporter;

  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  model = tflite::GetModel(model_gestures);
  // if (model->version() != TFLITE_SCHEMA_VERSION) {
  //   error_reporter->Report(
  //       "Model provided is schema version %d not equal "
  //       "to supported version %d.",
  //       model->version(), TFLITE_SCHEMA_VERSION);
  //   return;
  // }

  // This pulls in all the operation implementations we need.
  // NOLINTNEXTLINE(runtime-global-variables)
  static tflite::AllOpsResolver resolver;

  // Build an interpreter to run the model with.
  static tflite::MicroInterpreter static_interpreter(
      model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
  interpreter = &static_interpreter;

  // Allocate memory from the tensor_arena for the model's tensors.
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    error_reporter->Report("AllocateTensors() failed");
    return;
  }

  // Obtain pointers to the model's input and output tensors.
  input = interpreter->input(0);
  output = interpreter->output(0);

  // Keep track of how many inferences we have performed.
  //inference_count = 0;
}

// The name of this function is important for Arduino compatibility.
void loop() {

  // Vars de ejes
  int16_t gyr_x;
  int16_t gyr_y;
  int16_t gyr_z;

  // Lee 8 LSB de OUT_X
	gpio_clear(GPIOC, GPIO1);
	spi_send(SPI5, GYR_OUT_X_L | GYR_RNW);
	spi_read(SPI5);
	spi_send(SPI5, 0);
	gyr_x=spi_read(SPI5);
	gpio_set(GPIOC, GPIO1);

  // Lee 8 MSB de OUT_X
	gpio_clear(GPIOC, GPIO1);
	spi_send(SPI5, GYR_OUT_X_H | GYR_RNW);
	spi_read(SPI5);
	spi_send(SPI5, 0);
	gyr_x|=spi_read(SPI5) << 8;
	gpio_set(GPIOC, GPIO1);

  // Lee 8 LSB de OUT_Y
	gpio_clear(GPIOC, GPIO1);
	spi_send(SPI5, GYR_OUT_Y_L | GYR_RNW);
	spi_read(SPI5);
	spi_send(SPI5, 0);
	gyr_y=spi_read(SPI5);
	gpio_set(GPIOC, GPIO1);

  // Lee 8 MSB de OUT_Y
	gpio_clear(GPIOC, GPIO1);
	spi_send(SPI5, GYR_OUT_Y_H | GYR_RNW);
	spi_read(SPI5);
	spi_send(SPI5, 0);
	gyr_y|=spi_read(SPI5) << 8;
	gpio_set(GPIOC, GPIO1);

  // Lee 8 LSB de OUT_Z
	gpio_clear(GPIOC, GPIO1);
	spi_send(SPI5, GYR_OUT_Z_L | GYR_RNW);
	spi_read(SPI5);
	spi_send(SPI5, 0);
	gyr_z=spi_read(SPI5);
	gpio_set(GPIOC, GPIO1);

  // Lee 8 MSB de OUT_Z
	gpio_clear(GPIOC, GPIO1);
	spi_send(SPI5, GYR_OUT_Z_H | GYR_RNW);
	spi_read(SPI5);
	spi_send(SPI5, 0);
	gyr_z|=spi_read(SPI5) << 8;
	gpio_set(GPIOC, GPIO1);

  // Multiplicar por sensibilidad de gyro para 500dps
  gyr_x = gyr_x*L3GD20_SENSITIVITY_500DPS;
  gyr_y = gyr_y*L3GD20_SENSITIVITY_500DPS;
  gyr_z = gyr_z*L3GD20_SENSITIVITY_500DPS;

  // Calculate an x value to feed into the model. We compare the current
  // inference_count to the number of inferences per cycle to determine
  // our position within the range of possible x values the model was
  // trained on, and use this to calculate a value.
  // Place our calculated x value in the model's input tensor
  while (samplesRead < numSamples)
  {
    input->data.f[samplesRead * 3 + 0] = (gyr_x + 500.0) / 1000.0;
    input->data.f[samplesRead * 3 + 1] = (gyr_y + 500.0) / 1000.0;
    input->data.f[samplesRead * 3 + 2] = (gyr_z + 500.0) / 1000.0;

    samplesRead++;
  }
  
  

  if (samplesRead == numSamples)
  {
      // Run inference, and report any error
    TfLiteStatus invoke_status = interpreter->Invoke();
    if (invoke_status != kTfLiteOk) {
      error_reporter->Report("Invoke failed");
      return;
    }else{
      samplesRead = 0;
    }
  }
  
  

  // Read the predicted y value from the model's output tensor

  // Output the results. A custom HandleOutput function can be implemented
  // for each supported hardware target.
  //HandleOutput(error_reporter, gyr_x, gyr_y);
  //printf("gyr_xue: %.2f, gyr_yue: %.2f\n\r", gyr_x, gyr_y);
  //printf("gyr_xue: %.2f, gyr_yue: %.2f\n\r, gyr_zue: %.2f\n\r", gyr_x, gyr_y, gyr_z);

  // imprimir valores de giroscopio para verificar
	printf("X: "); my_usart_print_int(USART1, (gyr_x)); 
  printf("\tY: "); my_usart_print_int(USART1, (gyr_y)); 
  printf("\tZ: "); my_usart_print_int(USART1, (gyr_z)); 
  printf("\r\n");

  // Increment the inference_counter, and reset it if we have reached
  // the total number per cycle
  //inference_count += 1;
  for (int i = 0; i < NUM_GESTURES; i++)
  {
    printf(GESTURES[i]);
    printf(": "); my_usart_print_int(USART1, output->data.f[i]);
    printf("  ");
  }
  printf("\r\n");
}
