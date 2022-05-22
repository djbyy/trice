// Package src is a helper for testing the target C-code with CGO on the PC.
// Each called C function gets a Go wrapper which is used or tested in appropriate test functions.
// For some reason inside the trice_test.go an 'import "C"' is not possible.
package src

// #include <stdint.h>
// #include "trice_test.h"
// #include "trice.h"
// #cgo CFLAGS: -g -Wall -Iinc
import "C"
import (
	"unsafe"
)

// setTriceBuffer tells the underlying C code where to output the trice byte stream.
func setTriceBuffer(o []byte) {
	Cout := (*C.uint8_t)(unsafe.Pointer(&o[0]))
	C.SetTriceBuffer(Cout)
}

// setTriceFraming tells the underlying C code the trice byte stream framing method.
// 0 = none, 1 = COBS, 2 = TCOBS
func setTriceFraming(n int) {
	C.SetTriceFraming(C.int(n))
}

// triceCodeNoFraming performs trice code sequence n. It returns the actual byte stream length.
// It is simply a Go wrapper.
func triceCode(n int) int {
	return int(C.TriceCode(C.int(n)))
}
