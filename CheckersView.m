/*
 * Copyright (c) 2026 Jeandre Kruger
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#import <OpenGL/gl.h>
#import <OpenGL/glu.h>

#import "checkers.h"
#import "scenegraph.h"
#import "input.h"
#import "input_mapping-osx.h"
#import "text_input.h"
#import "CheckersView.h"

static int8_t keycode_buttons[128];

@implementation CheckersView

-(void)awakeFromNib {
	GLuint attribs[] = {
		NSOpenGLPFAWindow,
		NSOpenGLPFAAccelerated,
		NSOpenGLPFADoubleBuffer,
		NSOpenGLPFAColorSize, 24,
		NSOpenGLPFAAlphaSize, 8,
		NSOpenGLPFADepthSize, 24,
		NSOpenGLPFAAccumSize, 0,
		0
	};
	GLint vals[] = {1};
	NSOpenGLPixelFormat *fmt =
		[[NSOpenGLPixelFormat alloc]
		initWithAttributes:(NSOpenGLPixelFormatAttribute *)attribs]; 

	[[self window] setAcceptsMouseMovedEvents:YES];
	memset(keycode_buttons, -1, 128);
	for (int i = 0; i < NUM_BUTTONS; i++)
		keycode_buttons[input_mapping[i]] = i;

	[self setPixelFormat: fmt];

	/* Set vsync */
	[[self openGLContext] setValues:vals forParameter:NSOpenGLCPSwapInterval];
	
	[[self openGLContext] makeCurrentContext];
	sg_init([self bounds].size.width, [self bounds].size.height);
	checkers_init();
	
	/* See Apple Q&A 1385 */
	timer = [[NSTimer scheduledTimerWithTimeInterval:1.0/1000.0
			target:self selector:@selector(render:)
			userInfo:nil repeats:YES]
		retain];
}

-(BOOL)acceptsFirstResponder {
	return YES;
}

-(void)keyDown:(NSEvent *)evt {
	int button = keycode_buttons[[evt keyCode]];
	NSString *str = [evt characters];
	if (button >= 0)
		button_state[button] = 1;
	for (int i = 0; i < [str length]; i++) {
		unichar c = [str characterAtIndex:i];
		if (c < 128)
			text_input_add_char(c == 127 ? '\b' : c);
	}
}

-(void)keyUp:(NSEvent *)evt {
	int button = keycode_buttons[[evt keyCode]];
	if (button >= 0)
		button_state[button] = 0;
}

-(void)mouseUp:(NSEvent *)evt {
	NSPoint p = [evt locationInWindow];
	checkers_mouse_up(p.x, [self bounds].size.height - p.y);
}

-(void)mouseMoved:(NSEvent *)evt {
	NSPoint p = [evt locationInWindow];
	checkers_mouse_move(p.x, [self bounds].size.height - p.y);
}

-(void)render:(NSRect)rect {
	input_handle();
	checkers_update();
	[[self openGLContext] flushBuffer];
}
@end
