import struct
from collections import namedtuple
import random
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
    self.light = V3(0,0,1)
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
    self.zbuffer = [[-1e100 for x in range(self.width)] for y in range(self.height)]
  
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
  
  def triangle(self, A, B, C, color=color(0, 0, 0), normalized=False):
    if normalized:
      A = V3(int((A.x + 1)/2 * self.vpw + self.vpx), int((A.y + 1)/2 * self.vph + self.vpy), A.z)
      B = V3(int((B.x + 1)/2 * self.vpw + self.vpx), int((B.y + 1)/2 * self.vph + self.vpy), B.z)
      C = V3(int((C.x + 1)/2 * self.vpw + self.vpx), int((C.y + 1)/2 * self.vph + self.vpy), C.z)
    xmin, xmax, ymin, ymax = self.bbox(A, B, C)

    for x in range(xmin, xmax + 1):
      for y in range(ymin, ymax + 1):
        P = V2(x, y)
        w, v, u = self.barycentric(A, B, C, P)
        if w < 0 or v < 0 or u < 0:
          continue

        z = A.z * w + B.z * v + C.z * u
        if z > self.zbuffer[y][x]:
          self.point(x, y, color=color)
          self.zbuffer[y][x] = z

  def bbox(self, A, B, C):
    xmin = A.x
    xmax = A.x
    if B.x < xmin: xmin = B.x
    if C.x < xmin: xmin = C.x

    if B.x > xmax: xmax = B.x
    if C.x > xmax: xmax = C.x

    ymin = A.y
    ymax = A.y
    if B.y < ymin: ymin = B.y
    if C.y < ymin: ymin = C.y

    if B.y > ymax: ymax = B.y
    if C.y > ymax: ymax = C.y
    return xmin, xmax, ymin, ymax

  def cross(self, v0, v1):
    cx = v0.y * v1.z - v0.z * v1.y
    cy = v0.z * v1.x - v0.x * v1.z
    cz = v0.x * v1.y - v0.y * v1.x

    if cz == 0:
      pass
    return V3(cx, cy, cz)

  def barycentric(self, A, B, C, P):
    c = self.cross(V3(B.x - A.x, C.x - A.x, A.x - P.x), V3(B.y - A.y, C.y - A.y, A.y - P.y))
    cx = c.x
    cy = c.y
    cz = c.z

    if cz == 0: return 0, 0, 0

    u = cx / cz
    v = cy / cz
    w = 1 - u - v

    return w, v, u

  def sub(self, v0, v1):
    return V3(
      v0.x - v1.x,
      v0.y - v1.y,
      v0.z - v1.z
    )

  def length(self, v0):
    return (v0.x**2 + v0.y**2 + v0.z**2) ** 0.5
  
  def norm(self, v0):
    l = self.length(v0)
    if l == 0: return V3(0, 0, 0)

    return V3(
      v0.x/l,
      v0.y/l,
      v0.z/l
    )
  
  def dot(self, v0, v1):
    return v0.x * v1.x + v0.y * v1.y + v0.z * v1.z

  def load(self, filename, translate, scale):
    model = Obj(filename)
    tx = translate[0]
    ty = translate[1]
    sx = scale[0]
    sy = scale[1]
    def calcTriangle(A, B, C):
      dA = V3((A.x + tx)*sx, (A.y + ty)*sy, A.z * 1e90)
      dB = V3((B.x + tx)*sx, (B.y + ty)*sy, B.z * 1e90)
      dC = V3((C.x + tx)*sx, (C.y + ty)*sy, C.z * 1e90)
      normal = self.norm(self.cross(
        self.sub(B, A),
        self.sub(C, A)
      ))

      intensity = self.dot(normal, self.light)

      grey = round(255 * intensity)

      if intensity >= 0:
        self.triangle(dA, dB, dC, color=color(grey, grey, grey), normalized=True)

    for face in model.faces:
      vertexNum = len(face)
      
      if vertexNum == 3:
        f1 = face[0][0] - 1
        f2 = face[1][0] - 1
        f3 = face[2][0] - 1
        A = V3(model.vertices[f1][0], model.vertices[f1][1], model.vertices[f1][2])
        B = V3(model.vertices[f2][0], model.vertices[f2][1], model.vertices[f2][2])
        C = V3(model.vertices[f3][0], model.vertices[f3][1], model.vertices[f3][2])
        
        calcTriangle(A, B, C)

      elif vertexNum == 4:
        f1 = face[0][0] - 1
        f2 = face[1][0] - 1
        f3 = face[2][0] - 1
        f4 = face[3][0] - 1
        A = V3(model.vertices[f1][0], model.vertices[f1][1], model.vertices[f1][2])
        B = V3(model.vertices[f2][0], model.vertices[f2][1], model.vertices[f2][2])
        C = V3(model.vertices[f3][0], model.vertices[f3][1], model.vertices[f3][2])
        D = V3(model.vertices[f4][0], model.vertices[f4][1], model.vertices[f4][2])
        

        calcTriangle(A, B, C)
        calcTriangle(A, C, D)

      else:
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
  
  


#GLs start

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
glCreateWindow(1000, 1000)
glViewPort(50, 50, 900, 900)
glLoad("./models/model.obj", (0, 0), (0.75, 0.75))

glFinish()