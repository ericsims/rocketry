int g_winW     = 800;
int g_winH     = 600;

int g_maxArraySize   = 100;
float[] inputVals    = new float[g_maxArraySize];
cGraph g_graph       = new cGraph(100, 100, 600, 400);

void setup()
{
  size(g_winW, g_winH, P2D);

  for(int i=0; i<g_maxArraySize; ++i)
  {
    inputVals[i] = i%50;
  }
}

void draw()
{
  
  g_graph.drawGraphBox();
  g_graph.drawLine(inputVals, g_maxArraySize, 0, 100);
}

class cGraph
{
  float m_gWidth, m_gHeight;
  float m_gLeft, m_gBottom, m_gRight, m_gTop;
  
  cGraph(float x, float y, float w, float h)
  {
    m_gWidth     = w;
    m_gHeight    = h;
    m_gLeft      = x;
    m_gBottom    = g_winH - y;
    m_gRight     = x + w;
    m_gTop       = g_winH - y - h;
  }
  
  void drawGraphBox()
  {
    stroke(0, 0, 0);
    rectMode(CORNERS);
    rect(m_gLeft, m_gBottom, m_gRight, m_gTop);
  }
  
  void drawLine(float[] data, int dataSize, float minRange, float maxRange)
  {
    float graphMultX = m_gWidth/dataSize;
    float graphMultY = m_gHeight/(maxRange-minRange);
    
    stroke(255, 0, 0);
    for(int i=0; i<g_maxArraySize-1; ++i)
    {
      line(i*graphMultX+m_gLeft, m_gBottom-((data[i]-minRange)*graphMultY), (i+1)*graphMultX+m_gLeft, m_gBottom-((data[i+1]-minRange)*graphMultY));
    }
  }
}
