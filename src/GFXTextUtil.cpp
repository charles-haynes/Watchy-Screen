#include "GFXTextUtil.h"

#include <Adafruit_GFX.h>
#include <ctype.h>

#include "config.h" // DEBUG

/**
 * @brief when printing, how much width on the screen does this glyph require
 *
 * @param c char to compute width
 * @param f font in use
 * @return int16_t "width" of this character (actual width + inter char spacing)
 */
int16_t charWidth(const char c, const GFXfont *f) {
  if (c < f->first) {
    return 0;
  }
  if (c > f->last) {
    return 0;
  }
  const GFXglyph g = f->glyph[c - f->first];
  return g.xAdvance;
}

/**
 * @brief how far does this glyph extend, relative to the baseline?
 *
 * @param c character to compute
 * @param f font in use
 * @return int16_t how far above or below the baseline does this char extend?
 */
int16_t charDescent(const char c, const GFXfont *f) {
  if (c < f->first) {
    return 0;
  }
  if (c > f->last) {
    return 0;
  }
  const GFXglyph g = f->glyph[c - f->first];
  return g.yOffset + g.height;
}

/**
 * @brief draw word wrapped text within a bounding box. Takes GFX, bounding box,
 *        text string, and font
 *
 * @param g Adafruit_GFX to draw on
 * @param x upper left of bounding box (left)
 * @param y upper left of bounding box (top)
 * @param w width of bounding box
 * @param h height of bounding box
 * @param t text to draw. will draw as much as fits
 * @param f font to use
 */
void drawWordWrappedText(Adafruit_GFX &g, int16_t x, int16_t y, int16_t w,
                         int16_t h, const char *t, const GFXfont *f) {
  if (!t) {
    return;
  }
  g.setFont(f);
  g.setTextWrap(false);  // clip (GFX used to have a bug that would auto-wrap on
                         // exact width)
  const char *origT = t;
  const char *startLine = t;
  const char *wordBreak = t;
  int16_t wordBreakXPos = 0;
  int16_t xPos = 0;  ///< x position relative to left of bounding box
  int16_t yPos = f->yAdvance;  ///< y position relative to top of bounding box
  bool inWord = false;
  for (char c = *t;; c = *++t) {
    LOGD("%3d %3d/%3d %3d/%3d %2d %02x %1c", w, t-origT, xPos, wordBreak-origT, wordBreakXPos, charWidth(c, f), c, c);
    if (c == '\0' || c == '\n') { goto hardBreak; }
    xPos += charWidth(c,f);
    if (xPos <= w) {
      // this character fits within the width
      if (yPos+charDescent(c,f) > h) { return; }
      if (isSpace(c)) {
        inWord = false;
        wordBreak = t+1;
        wordBreakXPos = xPos;
      } else if (!inWord) {
        // start of a new word
        inWord = true;
        wordBreak = t;
        wordBreakXPos = xPos-charWidth(c,f);
      }
      continue;
    }
    // break this line
    if (startLine == wordBreak) {
    hardBreak:
      wordBreak = t;
      wordBreakXPos = xPos;
    }
    xPos -= wordBreakXPos;
    wordBreakXPos = 0; 
    g.setCursor(x, y+yPos);
    LOGD("%.*s", wordBreak-startLine, startLine);
    for (c=*startLine; startLine<wordBreak; c=*++startLine) { g.print(c); }
    yPos += f->yAdvance;
    if (c=='\0') {return;}
    if (c == '\n') {
      startLine++; // consume the \n
      inWord = false;
      xPos = 0;
      continue;
    }
    if (isSpace(c)) {
      // consume any remaining trailing whitespace
      while (isspace(*t) && *t != '\n') {t++;}
      if (*t == '\n') { t++; }
      startLine = t;
      wordBreak = startLine;
      xPos = 0;
    }
    xPos += charWidth(c,f);
  }
}
