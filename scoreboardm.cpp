


class glScoreBoard
{
  private:
  unsigned int lastScoreTick;
  uint tempScore;
  uint score;
  char tempText[255];
  
  public:

  glScoreBoard()
  {
    init();
  }
  
  void init()
  {
    tempScore=1;
    score=0;
    lastScoreTick = SDL_GetTicks();
  }
  
  void update(int point)
  {
    score=point;
  }
  
  void draw()
  {
    int dif=score - tempScore;

    if(tempScore != score )
    {
      if(lastScoreTick + 50 < SDL_GetTicks())
      {
        tempScore+= (float)dif/7.0 +1; ;
        if(tempScore > score)
          tempScore=score;
        lastScoreTick = SDL_GetTicks();

        sprintf(tempText, "%i", tempScore);
      }
    }
    
    glLoadIdentity();
    glTranslatef(-1.55, 1.24-(glText->getHeight(FONT_HIGHSCORE)/2.0), -3.0);
        
    glColor4f(1.0,1.0,1.0,1.0);
    glText->write(tempText, FONT_HIGHSCORE, 0, 1.0, 0.0, 0.0);
    
  }
};

