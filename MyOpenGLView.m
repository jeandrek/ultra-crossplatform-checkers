#import <OpenGL/gl.h>
#import <OpenGL/glu.h>

#import "checkers.h"
#import "scenegraph.h"
#import "input.h"
#import "MyOpenGLView.h"

//float angle = 0;

@implementation MyOpenGLView

-(void)awakeFromNib {
        GLuint attribs[] = 
        {
                //NSOpenGLPFANoRecovery,
                NSOpenGLPFAWindow,
                NSOpenGLPFAAccelerated,
                NSOpenGLPFADoubleBuffer,
                NSOpenGLPFAColorSize, 24,
                NSOpenGLPFAAlphaSize, 8,
                NSOpenGLPFADepthSize, 24,
                NSOpenGLPFAAccumSize, 0,
                0
        };

        NSOpenGLPixelFormat* fmt = [[NSOpenGLPixelFormat alloc] initWithAttributes: (NSOpenGLPixelFormatAttribute*) attribs]; 

	[self setPixelFormat: fmt];

	/* Set vsync */
	GLint vals[] = {1};
	[[self openGLContext] setValues:vals forParameter:NSOpenGLCPSwapInterval];
	
	[[self openGLContext] makeCurrentContext];
	sg_init([self bounds].size.width, [self bounds].size.height);
	checkers_init();
	
	/* See Apple Q&A 1385 */
	timer = [[NSTimer scheduledTimerWithTimeInterval: 1.0/1000.0
				target: self selector: @selector(render:) userInfo: nil repeats:YES]
			retain];
}

-(BOOL)acceptsFirstResponder {
	return YES;
}
/*
BOOL space_down = NO;
BOOL mouse_down = NO;

-(void)keyDown: (NSEvent *)evt {
	NSString *s = [evt characters];
	if ([s length] > 0 && [s characterAtIndex: 0] == ' ')
		space_down = YES;
}

-(void)keyUp: (NSEvent *)evt {
	NSString *s = [evt characters];
	if ([s length] > 0 && [s characterAtIndex: 0] == ' ')
		space_down = NO;
}

-(void)mouseDown: (NSEvent *)evt {
	mouse_down = YES;
}
*/

-(void)mouseUp: (NSEvent *)evt {
	//mouse_down = NO;
	NSPoint p = [evt locationInWindow];
	checkers_mouse_up(p.x, [self bounds].size.height - p.y);
}

-(void)render: (NSRect)rect {
	// input_handle();
	checkers_update();
	[[self openGLContext] flushBuffer];
}
@end
