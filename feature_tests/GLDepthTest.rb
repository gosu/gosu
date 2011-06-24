$LOAD_PATH << '../lib'
require 'gosu'
require 'rubygems'
require 'gl'
require 'glu'

include Gl
include Glu

class Test < Gosu::Window
  def initialize
    super(800, 600, false)
    self.caption = "oglt3"
  end

  def draw
    gl do
      # Background color to black
      glClearColor(0.0, 0.0, 0.0, 0)
      # Enables clearing of depth buffer
      glClearDepth(1.0)
      # Set type of depth test
      glDepthFunc(GL_LEQUAL)
      # Enable depth testing
      glEnable(GL_DEPTH_TEST)
      # Enable smooth color shading
      glShadeModel(GL_SMOOTH)

			glDisable(GL_CULL_FACE)

			glClearColor(0.0, 0.2, 0.5, 1.0)
			#glClearDepth(1)
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
	
      glMatrixMode(GL_MODELVIEW)
      glLoadIdentity
			glTranslate(0.0, 0.0, -10.0)
			glRotate(Gosu::milliseconds / 40.0, 0, 1, 0)

			glMatrixMode(GL_PROJECTION)
			glLoadIdentity
			gluPerspective(45.0, 1.2, 1.0, 100.0)


      glBegin(GL_POLYGON)
          # Draw front side of pyramid
          glColor3f(1.0, 0.0, 0.0)
          glVertex3f( 0.0,  1.0, 0.0)
          glColor3f(0.0, 1.0, 0.0)
          glVertex3f(-1.0, -1.0, 1.0)
          glColor3f(0.0, 0.0, 1.0)
          glVertex3f(1.0, -1.0, 1.0)
          # Draw right side of pyramid
          glColor3f(1.0, 0.0, 0.0)
          glVertex3f( 0.0,  1.0, 0.0)
          glColor3f(0.0, 0.0, 1.0)
          glVertex3f( 1.0, -1.0, 1.0)
          glColor3f(0.0, 1.0, 0.0)
          glVertex3f(1.0, -1.0, -1.0)
          # Draw back side of pyramid
          glColor3f(1.0, 0.0, 0.0)
          glVertex3f( 0.0,  1.0, 0.0)
          glColor3f(0.0, 0.0, 1.0)
          glVertex3f(1.0, -1.0, -1.0)
          glColor3f(0.0, 1.0, 0.0)
          glVertex3f(-1.0, -1.0, -1.0)
          # Draw left side of pyramid
          glColor3f(1.0, 0.0, 0.0)
          glVertex3f( 0.0,  1.0, 0.0)
          glColor3f(0.0, 0.0, 1.0)
          glVertex3f(-1.0, -1.0, -1.0)
          glColor3f(0.0, 1.0, 0.0)
          glVertex3f(-1.0, -1.0, 1.0)
      glEnd

			# glBegin(GL_TRIANGLES)
			#        glColor3f(1.0, 0.0, 0.0)
			#        glVertex3f(2.5, -2.5, -2.5)
			#        glColor3f(0.0, 1.0, 0.0)
			#        glVertex3f(2.5, -2.5, 2.5)
			#        glColor3f(0.0, 0.0, 1.0)
			#        glVertex3f(-2.5, -2.5, 2.5)
			# 
			#        glColor3f(1.0, 0.0, 0.0)
			#        glVertex3f(-2.5, -2.5, 2.5)
			#        glColor3f(0.0, 1.0, 0.0)
			#        glVertex3f(-2.5, -2.5, -2.5)
			#        glColor3f(0.0, 0.0, 1.0)
			#        glVertex3f(2.5, -2.5, -2.5)
			# 
			#        glColor3f(1.0, 0.0, 0.0)
			#        glVertex3f(-2.5, -2.5, -2.5)
			#        glColor3f(0.0, 1.0, 0.0)
			#        glVertex3f(-2.5, -2.5, 2.5)
			#        glColor3f(0.0, 0.0, 1.0)
			#        glVertex3f(-2.5, 2.5, 2.5)
			# 
			#        glColor3f(1.0, 0.0, 0.0)
			#        glVertex3f(-2.5, 2.5, 2.5)
			#        glColor3f(0.0, 1.0, 0.0)
			#        glVertex3f(-2.5, 2.5, -2.5)
			#        glColor3f(0.0, 0.0, 1.0)
			#        glVertex3f(-2.5, -2.5, -2.5)
			# 
			#        glColor3f(1.0, 0.0, 0.0)
			#        glVertex3f(-2.5, 2.5, -2.5)
			#        glColor3f(0.0, 1.0, 0.0)
			#        glVertex3f(-2.5, 2.5, 2.5)
			#        glColor3f(0.0, 0.0, 1.0)
			#        glVertex3f(2.5, 2.5, 2.5)
			# 
			#        glColor3f(1.0, 0.0, 0.0)
			#        glVertex3f(2.5, 2.5, 2.5)
			#        glColor3f(0.0, 1.0, 0.0)
			#        glVertex3f(2.5, 2.5, -2.5)
			#        glColor3f(0.0, 0.0, 1.0)
			#        glVertex3f(-2.5, 2.5, -2.5)
			# 
			#        glColor3f(1.0, 0.0, 0.0)
			#        glVertex3f(2.5, 2.5, -2.5)
			#        glColor3f(0.0, 1.0, 0.0)
			#        glVertex3f(2.5, 2.5, 2.5)
			#        glColor3f(0.0, 0.0, 1.0)
			#        glVertex3f(2.5, -2.5, 2.5)
			# 
			#        glColor3f(1.0, 0.0, 0.0)
			#        glVertex3f(2.5, -2.5, 2.5)
			#        glColor3f(0.0, 1.0, 0.0)
			#        glVertex3f(2.5, -2.5, -2.5)
			#        glColor3f(0.0, 0.0, 1.0)
			#        glVertex3f(2.5, 2.5, -2.5)
			# 
			#        glColor3f(1.0, 0.0, 0.0)
			#        glVertex3f(-2.5, -2.5, 2.5)
			#        glColor3f(0.0, 1.0, 0.0)
			#        glVertex3f(2.5, -2.5, 2.5)
			#        glColor3f(0.0, 0.0, 1.0)
			#        glVertex3f(2.5, 2.5, 2.5)
			# 
			#        glColor3f(1.0, 0.0, 0.0)
			#        glVertex3f(2.5, 2.5, 2.5)
			#        glColor3f(0.0, 1.0, 0.0)
			#        glVertex3f(-2.5, 2.5, 2.5)
			#        glColor3f(0.0, 0.0, 1.0)
			#        glVertex3f(-2.5, -2.5, 2.5)
			# 
			#        glColor3f(1.0, 0.0, 0.0)
			#        glVertex3f(2.5, -2.5, -2.5)
			#        glColor3f(0.0, 1.0, 0.0)
			#        glVertex3f(-2.5, -2.5, -2.5)
			#        glColor3f(0.0, 0.0, 1.0)
			#        glVertex3f(-2.5, 2.5, -2.5)
			# 
			#        glColor3f(1.0, 0.0, 0.0)
			#        glVertex3f(-2.5, 2.5, -2.5)
			#        glColor3f(0.0, 1.0, 0.0)
			#        glVertex3f(2.5, 2.5, -2.5)
			#        glColor3f(0.0, 0.0, 1.0)
			#        glVertex3f(2.5, -2.5, -2.5)
			#      glEnd
			

		end
  end

  def button_down(id)
    if Gosu::Button::KbEscape == id
      close
    end
  end
end

Test.new.show
