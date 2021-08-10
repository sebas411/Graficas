import struct

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
  
  def line(self, x0, y0, x1, y1):

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

    points = []
    for x in range(x0, x1, 1 if x0 < x1 else -1):

      if steep:
        points.append((y, x))
      else:
        points.append((x, y))

      offset += 2 * dy

      if offset >= threshold:
        if y0 < y1: y += 1
        else: y -= 1
        threshold += 2 * dx
      
    for point in points:
      self.point(*point)
  
  def fill(self):
    lastFull = False

    frameBuffer1 = [[self.clear_color for x in range(self.width)] for y in range(self.height)]
    frameBuffer2 = [[self.clear_color for x in range(self.width)] for y in range(self.height)]
    frameBuffer3 = [[self.clear_color for x in range(self.width)] for y in range(self.height)]
    frameBuffer4 = [[self.clear_color for x in range(self.width)] for y in range(self.height)]

    for x in range(self.width):
      for y in range(self.height):
        frameBuffer1[y][x] = self.framebuffer[y][x]
        frameBuffer2[y][x] = self.framebuffer[y][x]
        frameBuffer3[y][x] = self.framebuffer[y][x]
        frameBuffer4[y][x] = self.framebuffer[y][x]

    #left to right
    for y in range(self.height):
      interior = False
      for x in range(self.width):
        if frameBuffer1[y][x] != WHITE:
          if not lastFull:
            interior = not interior
          lastFull = True
        else:
          lastFull = False
          if interior:
            frameBuffer1[y][x] = self.current_color

    #right to left
    for y in range(self.height):
      interior = False
      for x in range(self.width-1, -1, -1):
        if frameBuffer2[y][x] != WHITE:
          if not lastFull:
            interior = not interior
          lastFull = True
        else:
          lastFull = False
          if interior:
            frameBuffer2[y][x] = self.current_color
    
    #bottom to top
    for x in range(self.width):
      interior = False
      for y in range(self.height):
        if frameBuffer3[y][x] != WHITE:
          if not lastFull:
            interior = not interior
          lastFull = True
        else:
          lastFull = False
          if interior:
            frameBuffer3[y][x] = self.current_color

    #top to bottom
    for x in range(self.width):
      interior = False
      for y in range(self.height-1, -1, -1):
        if frameBuffer4[y][x] != WHITE:
          if not lastFull:
            interior = not interior
          lastFull = True
        else:
          lastFull = False
          if interior:
            frameBuffer4[y][x] = self.current_color

    for x in range(self.width):
      for y in range(self.height):
        agrees = 0
        agreeColor = None
        if frameBuffer1[y][x] == frameBuffer2[y][x] == frameBuffer3[y][x]:
          agrees += 1
          agreeColor = frameBuffer1[y][x]
        if frameBuffer2[y][x] == frameBuffer3[y][x] == frameBuffer4[y][x]:
          agrees += 1
          agreeColor = frameBuffer2[y][x]
        if frameBuffer3[y][x] == frameBuffer4[y][x] == frameBuffer1[y][x]:
          agrees += 1
          agreeColor = frameBuffer3[y][x]
        if frameBuffer4[y][x] == frameBuffer1[y][x] == frameBuffer2[y][x]:
          agrees += 1
          agreeColor = frameBuffer4[y][x]

        if agrees >= 1:
          self.framebuffer[y][x] = agreeColor

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
    r.line(x0, y0, x1, y1)
  else:
    print("Bad window")

def glFinish():
  if r and r.width > 0 and r.height > 0:
    r.render()
  else:
    print("Bad window")

glInit()
glCreateWindow(1001, 1001)
glViewPort(0, 0, 1000, 1000)

glClearColor(1, 1, 1)
glClear()

glColor(1, 0, 0)

poligons = [
  [(165, 380), (185, 360), (180, 330), (207, 345), (233, 330), (230, 360), (250, 380), (220, 385), (205, 410), (193, 383)],
  [(321, 335), (288, 286), (339, 251), (374, 302)],
  [(377, 249), (411, 197), (436, 249)],
  [(413, 177), (448, 159), (502, 88), (553, 53), (535, 36), (676, 37), (660, 52), (750, 145), (761, 179), (672, 192), (659, 214), (615, 214), (632, 230), (580, 230), (597, 215), (552, 214), (517, 144), (466, 180)],
  [(682, 175), (708, 120), (735, 148), (739, 170)],
]

for poligon in poligons:
  for i in range(len(poligon)):
    glLine(poligon[i][0], poligon[i][1], poligon[(i + 1) % len(poligon)][0], poligon[(i + 1) % len(poligon)][1])


r.fill()
glFinish()