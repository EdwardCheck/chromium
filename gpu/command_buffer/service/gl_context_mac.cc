// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file implements the ViewGLContext and PbufferGLContext classes.

#include "gpu/command_buffer/service/gl_context.h"
#include "gpu/command_buffer/common/logging.h"

#if !defined(UNIT_TEST)
#include "app/surface/accelerated_surface_mac.h"
#endif

namespace gpu {

bool ViewGLContext::Initialize(bool multisampled) {
#if !defined(UNIT_TEST)
  if (multisampled) {
    DLOG(WARNING) << "Multisampling not implemented.";
  }

  if (!surface_->Initialize()) {
    DLOG(ERROR) << "Error initializing accelerated surface.";
    return false;
  }

  if (!MakeCurrent()) {
    Destroy();
    DLOG(ERROR) << "Couldn't make context current for initialization.";
    return false;
  }

  if (!InitializeGLEW()) {
    Destroy();
    return false;
  }

  return true;
#else
  return true;
#endif  // UNIT_TEST
}

void ViewGLContext::Destroy() {
#if !defined(UNIT_TEST)
  surface_->Destroy();
#endif  // UNIT_TEST
}

bool ViewGLContext::MakeCurrent() {
#if !defined(UNIT_TEST)
  return surface_->MakeCurrent();
#else
  return true;
#endif
}

bool ViewGLContext::IsOffscreen() {
  return false;
}

void ViewGLContext::SwapBuffers() {
#if !defined(UNIT_TEST)
  surface_->SwapBuffers();
#endif  // UNIT_TEST
}

gfx::Size ViewGLContext::GetSize() {
#if !defined(UNIT_TEST)
  return surface_->GetSize();
#else
  return gfx::Size();
#endif  // UNIT_TEST
}

GLContextHandle ViewGLContext::GetHandle() {
#if !defined(UNIT_TEST)
  return surface_->context();
#else
  return NULL;
#endif  // UNIT_TEST
}

bool PbufferGLContext::Initialize(GLContext* shared_context) {
#if !defined(UNIT_TEST)
  // Get the shared context handle.
  GLContextHandle shared_handle = NULL;
  if (shared_context)
    shared_handle = shared_context->GetHandle();

  // Create a 1x1 pbuffer and associated context to bootstrap things.
  static const CGLPixelFormatAttribute attribs[] = {
    (CGLPixelFormatAttribute) kCGLPFAPBuffer,
    (CGLPixelFormatAttribute) 0
  };
  CGLPixelFormatObj pixel_format;
  GLint num_pixel_formats;
  if (CGLChoosePixelFormat(attribs,
                           &pixel_format,
                           &num_pixel_formats) != kCGLNoError) {
    DLOG(ERROR) << "Error choosing pixel format.";
    Destroy();
    return false;
  }
  if (!pixel_format) {
    return false;
  }
  CGLError res = CGLCreateContext(pixel_format, shared_handle, &context_);
  CGLDestroyPixelFormat(pixel_format);
  if (res != kCGLNoError) {
    DLOG(ERROR) << "Error creating context.";
    Destroy();
    return false;
  }
  if (CGLCreatePBuffer(1, 1,
                       GL_TEXTURE_2D, GL_RGBA,
                       0, &pbuffer_) != kCGLNoError) {
    DLOG(ERROR) << "Error creating pbuffer.";
    Destroy();
    return false;
  }
  if (CGLSetPBuffer(context_, pbuffer_, 0, 0, 0) != kCGLNoError) {
    DLOG(ERROR) << "Error attaching pbuffer to context.";
    Destroy();
    return false;
  }

  if (!MakeCurrent()) {
    Destroy();
    DLOG(ERROR) << "Couldn't make context current for initialization.";
    return false;
  }

  if (!InitializeGLEW()) {
    Destroy();
    return false;
  }
#endif  // UNIT_TEST

  return true;
}

void PbufferGLContext::Destroy() {
#if !defined(UNIT_TEST)
  if (context_) {
    CGLDestroyContext(context_);
    context_ = NULL;
  }

  if (pbuffer_) {
    CGLDestroyPBuffer(pbuffer_);
    pbuffer_ = NULL;
  }
#endif  // UNIT_TEST
}

bool PbufferGLContext::MakeCurrent() {
#if !defined(UNIT_TEST)
  if (CGLGetCurrentContext() != context_) {
    if (CGLSetCurrentContext(context_) != kCGLNoError) {
      DLOG(ERROR) << "Unable to make gl context current.";
      return false;
    }
  }
#endif  // UNIT_TEST

  return true;
}

bool PbufferGLContext::IsOffscreen() {
  return true;
}

void PbufferGLContext::SwapBuffers() {
  NOTREACHED() << "Cannot call SwapBuffers on a PbufferGLContext.";
}

gfx::Size PbufferGLContext::GetSize() {
  NOTREACHED() << "Should not be requesting size of a PbufferGLContext.";
  return gfx::Size(1, 1);
}

GLContextHandle PbufferGLContext::GetHandle() {
#if !defined(UNIT_TEST)
  return context_;
#else
  return NULL;
#endif  // UNIT_TEST
}

}  // namespace gpu
