import struct
from collections import namedtuple
from obj import Obj

V2 = namedtuple('vertex2d', ['x', 'y'])
V3 = namedtuple('vertex3d', ['x', 'y', 'z'])

r = None

def char(c):
  return struct.pack('=c', c.encode('ascii'))

def hword(w):
  #short
  return struct.pack('=h', w)

def word(w):
  #long
  return struct.pack('=l', w)

def color(r, g, b):
  return bytes([b, g, r])

BLACK = color(0, 0, 0)
WHITE = color(255, 255, 255)

class Renderer(object):
  def __init__(self):
    self.width = 0
    self.height = 0
    self.current_color = WHITE
    self.clear_color = BLACK
    self.vph = 0
    self.vpw = 0
    self.vpx = 0
    self.vpy = 0

  def createWindow(self, width, height):
    self.width = width
    self.height = height
    self.clear()

  def setViewPort(self, x, y, width, height):
    self.vph = height
    self.vpw = width
    self.vpx = x
    self.vpy = y

  def setCurrentColor(self, color):
    self.current_color = color

  def clear(self):
    self.framebuffer = [[self.clear_color for x in range(self.width)] for y in range(self.height)]
  
  def setClearColor(self, color):
    self.clear_color = color
  
  def write(self, filename):
    padding = self.width % 4


    f = open(filename, 'bw')

    # header 14
    f.write(char('B'))
    f.write(char('M'))
    f.write(word(14+40+3*(self.width*self.height)))
    f.write(word(0))
    f.write(word(14+40))

    # infoHeader 40
    f.write(word(40))
    f.write(word(self.width))
    f.write(word(self.height))
    f.write(hword(1))
    f.write(hword(24))
    f.write(word(0))
    f.write(word(3*(self.width*self.height)))
    f.write(word(0))
    f.write(word(0))
    f.write(word(0))
    f.write(word(0))

    # bitmap
    for y in range(self.height):
      for x in range(self.width):
        f.write(self.framebuffer[y][x])
      for p in range(padding):
        f.write(struct.pack('=x'))


    f.close()
  
  def render(self):
    self.write('image.bmp')

  def point(self, x, y, color = None):
    self.framebuffer[y][x] = color or self.current_color

  def drawVertex(self, x, y):
    x01 = (x+1)/2
    y01 = (y+1)/2
    windowX = int(x01 * self.vpw + self.vpx)
    windowY = int(y01 * self.vph + self.vpy)
    self.point(windowX, windowY)
  
  def deNormalizedLine(self, x0, y0, x1, y1):
    dy = abs(y1 - y0)
    dx = abs(x1 - x0)
    steep = dy > dx

    if steep:
      x0, y0 = y0, x0
      x1, y1 = y1, x1 #swap en c++
      dy, dx = dx, dy


    offset = 0
    threshold =  dx
    y = y0

    for x in range(x0, x1, 1 if x1 > x0 else -1):

      if steep:
        self.point(y, x)
      else:
        self.point(x, y)

      offset += 2 * dy

      if offset >= threshold:
        if y0 < y1: y += 1
        else: y -= 1
        threshold += 2 * dx
  
  def vertexLine(self, A, B):
    x0 = A.x
    x1 = B.x
    y0 = A.y
    y1 = B.y
    self.deNormalizedLine(x0, y0, x1, y1)

  def line(self, nx0, ny0, nx1, ny1):
    x0 = int((nx0+1)/2 * self.vpw + self.vpx)
    x1 = int((nx1+1)/2 * self.vpw + self.vpx)
    y0 = int((ny0+1)/2 * self.vph + self.vpy)
    y1 = int((ny1+1)/2 * self.vph + self.vpy)
    self.deNormalizedLine(x0, x1, y0, y1)

  def triangleWireframe(self, A, B, C):
    self.vertexLine(A, B)
    self.vertexLine(B, C)
    self.vertexLine(C, A)

  def triangle(self, A, B, C):
    if A.y > B.y:
      A, B = B, A

    if A.y > C.y:
      A, C = C, A

    if B.y > C.y:
      B, C = C, B

    dx_ac = C.x - A.x
    dy_ac = C.y - A.y
    mi_ac = dx_ac/dy_ac

    dx_ab = B.x - A.x
    dy_ab = B.y - A.y
    
    mi_ab = dx_ab/dy_ab

    for y in range(A.y, B.y +1):
      xi = round(A.x - mi_ac * (A.y - y))
      xf = round(A.x - mi_ab * (A.y - y))

      if xi > xf:
        xi, xf = xf, xi

      for x in range(xi, xf + 1):
        self.point(x, y)

    dx_bc = C.x - B.x
    dy_bc = C.y - B.y
    mi_bc = dx_bc/dy_bc

    for y in range(B.y, C.y + 1):
      xi = round(A.x - mi_ac * (A.y - y))
      xf = round(B.x - mi_bc * (B.y - y))

      if xi > xf:
        xi, xf = xf, xi

      for x in range(xi, xf + 1):
        self.point(x, y)

  def load(self, filename, translate, scale):
    model = Obj(filename)

    for face in model.faces:
      vertexNum = len(face)
      for j in range(vertexNum):
        f1 = face[j][0]
        f2 = face[(j+1) % vertexNum][0]

        v1 = model.vertices[f1 - 1]
        v2 = model.vertices[f2 - 1]

        x1 = (v1[0] + translate[0]) * scale[0]
        y1 = (v1[1] + translate[1]) * scale[1]
        x2 = (v2[0] + translate[0]) * scale[0]
        y2 = (v2[1] + translate[1]) * scale[1]
        if (x1 >= -1 and y1 >= -1 and x2 >= -1 and y2 >= -1) and (x1 <= 1 and y1 <= 1 and x2 <= 1 and y2 <= 1):
          self.line(x1, y1, x2, y2)
  

def glInit():
  global r
  r = Renderer()

def glCreateWindow(width, height):
  if r:
    if width > 0 and height > 0:
      r.createWindow(width, height)
    else:
      print("Invalid values for window size")
  else:
    print("Missing initialization")

def glViewPort(x, y, width, height):
  if r and r.width > 0 and r.height > 0:
    if (x + width > r.width or y + height > r.height) and (x > 0 and y > 0 and width > 0 and height > 0):
      print("Cannot set viewport bigger than window")
    r.setViewPort(x, y, width, height)
  else:
    print("Bad window")

def glClear():
  if r and r.width > 0 and r.height > 0:
    r.clear()
  else:
    print("Bad window")

def glClearColor(R, G, B):
  if r and r.width > 0 and r.height > 0:
    if (R >= 0 and G >= 0 and B >= 0) and (R <= 1 and G <= 1 and B <= 1):
      r.setClearColor(color(int(R*255), int(G*255), int(B*255)))
    else:
      print("Invalid values for color")
  else:
    print("Bad window")

def glVertex(x, y):
  if r and r.width > 0 and r.height > 0:
    if (x >= -1 and y >= -1) and (x <= 1 and y <= 1):
      r.drawVertex(x, y)
    else:
      print("Cannot draw outside of viewport")
  else:
    print("Bad window")

def glColor(R, G, B):
  if r and r.width > 0 and r.height > 0:
    if (R >= 0 and G >= 0 and B >= 0) and (R <= 1 and G <= 1 and B <= 1):
      r.setCurrentColor(color(int(R*255), int(G*255), int(B*255)))
    else:
      print("Invalid values for color")
  else:
    print("Bad window")

def glLine(x0, y0, x1, y1):
  if r and r.width > 0 and r.height > 0:
    if (x0 >= -1 and y0 >= -1 and x1 >= -1 and y1 >= -1) and (x0 <= 1 and y0 <= 1 and x1 <= 1 and y1 <= 1):
      r.line(x0, y0, x1, y1)
    else:
      print("Invalid values for line limits")
  else:
    print("Bad window")

def glLoad(filename, translate, scale):
  if r and r.width > 0 and r.height > 0:
    r.load(filename, translate, scale)
  else:
    print("Bad window")

def glFinish():
  if r and r.width > 0 and r.height > 0:
    r.render()
  else:
    print("Bad window")





glInit()
glCreateWindow(200, 200)
glViewPort(0, 0, 200, 200)
r.triangle(V2(10, 70),  V2(50, 160), V2(70, 80))
r.triangle(V2(180, 50), V2(150, 1),  V2(70, 180))
r.triangle(V2(180, 150), V2(120, 160), V2(130, 180))

glFinish()