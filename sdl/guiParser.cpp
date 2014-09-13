/* This is simple demonstration of how to use expat. This program
   reads an XML document from standard input and writes a line with
   the name of each element to standard output indenting child
   elements by one tab stop more than their parent element.
   It must be used with Expat compiled for UTF-8 output.
*/

#include <stdio.h>
#include <string.h>
#include <expat.h>
#include "gui.h"

#define WORK_BUF_SIZE 256
char workingBuf[WORK_BUF_SIZE];
int workingIndex = 0;

/* Head of the game information linked list */
gameInfo_t *gameInfo = NULL;

/* Temp nodes used in game information linked list */
gameInfo_t *prevGame = NULL;
gameInfo_t *currentGame = NULL;

/* Define the XML tags used in the games.xml */
typedef enum {
  /* Start! */
  TAG_FIRST = 0,
  TAG_SNES = 0,
  TAG_GAME,
  TAG_TITLE,
  TAG_ROM,
  TAG_IMAGE,
  TAG_YEAR,
  /* There can be multiple of these per "game" tag */
  TAG_GENRE,
  TAG_TEXT,
  /* Done! */
  TAG_LAST
} TagUsed_t;

typedef struct {
  const char *name; /* Name of the tag */
  const int depth;  /* Proper depth for tag use */
} TagInfo_t;

static TagInfo_t tagInfo[TAG_LAST] = { 
  { "snes",  1 }, /* TAG_SNES */
  { "game",  2 }, /* TAG_GAME */
  { "title", 3 }, /* TAG_TITLE */
  { "rom",   3 }, /* TAG_ROM */
  { "image", 3 }, /* TAG_IMAGE */
  { "year",  3 }, /* TAG_YEAR */
  { "genre", 3 }, /* TAG_GENRE */
  { "text",  3 }  /* TAG_TEXT */
};

/* Flag for what tag we're currently in */
static char inTagFlag[TAG_LAST];
/* Flags for the fields that have defined in this gameInfo node */
static char definedTagFlag[TAG_LAST];

/* Counters for how many of a tag have been used in this "game" tag */
static int currentGenre = 0;
static int currentText = 0;

static void XMLCALL
characterData(void *userData, const char *data, int len)
{
  int i;

  /* Range check */
  if ((len + workingIndex + 1) > WORK_BUF_SIZE)
  {
    fprintf(stderr, "ERROR: Text is way too long in tag, truncating\n");
    len = WORK_BUF_SIZE - workingIndex - 1;
  }

  for (i=0; i < len; i++)
  {
    workingBuf[workingIndex] = data[i];
    //fprintf(stderr, "%d[%c]", workingIndex, data[i]);
    workingIndex++;
  }
}

static void XMLCALL
startElement(void *userData, const char *name, const char **atts)
{
  int i;
  int *depthPtr = (int *)userData;

  /* Increase depth */
  *depthPtr += 1;

  memset(workingBuf, 0, WORK_BUF_SIZE);
  workingIndex = 0;

  for (i = TAG_FIRST; i < TAG_LAST; i++)
  {
    if (
      !strcasecmp(name, tagInfo[i].name) && /* Tag match? */ 
      (tagInfo[i].depth == *depthPtr)       /* Depth match? */ 
    )
    {
      /* Check to make sure this tag isn't open */
      if (!inTagFlag[i])
        /* We're in a tag, now */
        inTagFlag[i] = 1;
      else
      {
        fprintf(stderr, "ERROR: <%s> tag found when one already open\n", tagInfo[i].name);
      }
      /* Switch to the proper rule for the special tags */
      switch(i)
      {
        case TAG_GAME:
          /* Allocate a new info node */
fprintf(stderr, "\ncalloc() new node: <%s>", tagInfo[i].name);
          prevGame = currentGame;
          currentGame->next = (gameInfo_t *)calloc(1, sizeof(gameInfo_t));
          currentGame = currentGame->next;
          currentGame->prev = prevGame;
          /* Defaults */
          strcpy(currentGame->dateText, DEFAULT_DATE_TEXT);
          currentGenre = 0;
          currentText = 0;
          break;

        case TAG_GENRE:
          fprintf(stderr, "<%s>", tagInfo[i].name);
          break;

        case TAG_TEXT:
          fprintf(stderr, "<%s>", tagInfo[i].name);
          break;
        default:
          fprintf(stderr, "<%s>", tagInfo[i].name);
          break;
      }
    }
  }
}

static void XMLCALL
endElement(void *userData, const char *name)
{
  int *depthPtr = (int *)userData;
  int i, x, invalidTag = 0;

  for (i = TAG_FIRST; i < TAG_LAST; i++)
  {
    if (
      !strcasecmp(name, tagInfo[i].name) && /* Tag match? */
      (tagInfo[i].depth == *depthPtr)       /* Depth match? */
    )
    {
      /* Is this the current, open tag? */
      if (inTagFlag[i])
      {
        /* Close it */
        inTagFlag[i] = 0;
        fprintf(stderr, "</%s>", tagInfo[i].name);

        switch(i) {
          case TAG_GAME:
            /* Are any tags still open? */
            for (x = TAG_GAME + 1; x < TAG_LAST; x++)
              if (inTagFlag[x]) invalidTag = 1; 
            if (invalidTag)
            {
              fprintf(stderr, "ERROR: Tried to close <game> tag while another tag was open\n");
              if (currentGame) free(currentGame);
              currentGame = prevGame;
              prevGame = currentGame->prev;
              break;
            }
            /* Do we have the bare minimum fields? */
            else if (definedTagFlag[TAG_TITLE] && definedTagFlag[TAG_ROM])
            {
              /* TODO: Store this */
              totalGames++;
            }
            else 
            {
              fprintf(stderr, "ERROR: Missing game title or rom filename\n");
              if (currentGame) free(currentGame);
              currentGame = prevGame;
              prevGame = currentGame->prev;
              
            }

            /* Reset the flags for tag in use and defined */
            for (i = TAG_FIRST; i < TAG_LAST; i++) {
              inTagFlag[i] = 0;
              definedTagFlag[i] = 0;
            }
            break;

          case TAG_TITLE:
            if (definedTagFlag[i])
            {
              fprintf(stderr, "ERROR: Title already defined for game\n");
              break;
            }
fprintf(stderr, "Copying gameTitle '%s' into node\n", workingBuf);
            strncpy(currentGame->gameTitle, workingBuf, GAME_TITLE_SIZE - 1);
            definedTagFlag[i] = 1;
            break;

          case TAG_ROM:
            if (definedTagFlag[i])
            {
              fprintf(stderr, "ERROR: ROM already defined for game\n");
              break;
            }
            fprintf(stderr, "Copying romFile '%s' into node\n", workingBuf);
            strncpy(currentGame->romFile, workingBuf, ROM_FILE_SIZE - 1);
            definedTagFlag[i] = 1;
            break;

          case TAG_IMAGE:
            if (definedTagFlag[i])
            {
              fprintf(stderr, "ERROR: Image already defined for game\n");
              break;
            }
            fprintf(stderr, "Copying imageFile '%s' into node\n", workingBuf);
            strncpy(currentGame->imageFile, workingBuf, IMAGE_FILE_SIZE -1);
            definedTagFlag[i] = 1;
            break;

          case TAG_YEAR:
            if (definedTagFlag[i])
            {
              fprintf(stderr, "ERROR: Year already defined for game\n");
              break;
            }
            fprintf(stderr, "Copying dateText '%s' into node\n", workingBuf);
            strncpy(currentGame->dateText, workingBuf, DATE_TEXT_SIZE -1);
            definedTagFlag[i] = 1;
            break;

          case TAG_GENRE:
            if (currentGenre < MAX_GENRE_TYPES)
            {
fprintf(stderr, "Copying genreText[%d] '%s' into node\n", currentGenre, workingBuf);
              strncpy(currentGame->genreText[currentGenre], workingBuf, GENRE_TEXT_SIZE - 1);
              currentGenre++;
            }
            definedTagFlag[i] = 1;
            break;

          case TAG_TEXT:
            if (currentText < MAX_TEXT_LINES)
            {
fprintf(stderr, "Copying infoText[%d] '%s' into node\n", currentText, workingBuf);
              strncpy(currentGame->infoText[currentText], workingBuf, INFO_TEXT_SIZE - 1);
              currentText++;
            }
            definedTagFlag[i] = 1;
            break;

          default:
            fprintf(stderr, "ERROR: Close of undefined tag\n");
            break;
        } /* end switch */
      }
      else
        fprintf(stderr, "Closing tag mismatch: <\\%s> found\n", tagInfo[i].name);
    }
  }
  /* Decrease depth */
  *depthPtr -= 1;
}

int loadGameConfig(void)
{
  char buf[BUFSIZ];
  XML_Parser parser = XML_ParserCreate(NULL);
  int done, i, depth = 0;
  FILE *config = NULL;

  totalGames = 0;

  XML_SetUserData(parser, &depth);
  XML_SetElementHandler(parser, startElement, endElement);
  XML_SetCharacterDataHandler(parser, characterData);

  config = fopen("/boot/uboot/beaglesnes/games.xml", "r");
  if (!config) {
    fprintf(stderr, "Unable to open game configuration file games.xml\n");
    return 1;
  }

  /* Initialize the flags for tag in use and defined */
  for (i = TAG_FIRST; i < TAG_LAST; i++) {
    inTagFlag[i] = 0;
    definedTagFlag[i] = 0;
  }

  /* Initialize the dummy head sentinel */
  fprintf(stderr, "Creating sentinel\n");
  gameInfo = (gameInfo_t *)calloc(1, sizeof(gameInfo_t)); 
  currentGame = gameInfo;

  do {
    int len = (int)fread(buf, 1, sizeof(buf), config);
    done = len < sizeof(buf);
    if (XML_Parse(parser, buf, len, done) == XML_STATUS_ERROR) {
      fprintf(stderr,
              "%s at line %lu\n",
              XML_ErrorString(XML_GetErrorCode(parser)),
              XML_GetCurrentLineNumber(parser));
      fclose(config);
      return 1;
    }
  } while (!done);
  XML_ParserFree(parser);
  fclose(config);
  return 0;
}
#if 0 // AWH
int main(void)
{
  int x, i = 1;
  loadGameConfig();

  currentGame = gameInfo;
  while(currentGame->next)
  {
    currentGame = currentGame->next;
    fprintf(stderr, "Node %d:\n", i);
    fprintf(stderr, "  Title: '%s'\n", currentGame->gameTitle);
    fprintf(stderr, "  Rom: '%s'\n", currentGame->romFile);
    fprintf(stderr, "  Image: '%s'\n", currentGame->imageFile);
    fprintf(stderr, "  Date: '%s'\n", currentGame->dateText);
    for (x=0; x < MAX_GENRE_TYPES; x++)
    {
      fprintf(stderr, "  Genre%d: '%s'\n", x, currentGame->genreText[x]);
    }
    for (x = 0; x < MAX_TEXT_LINES; x++)
    {
      fprintf(stderr, "  Text%d: '%s'\n", x, currentGame->infoText[x]);
    }
    i++;
  }  
}
#endif // AWH
