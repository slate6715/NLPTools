/* 
 * File:   WordNet.cpp
 * Author: root
 * 
 * Created on October 4, 2012, 3:26 PM
 */


 // Precompiled headers
#ifdef _WIN32
#include "stdafx.h"
#endif

// #using namespace util;

#ifdef USE_WORDNET

#include "WordNet.h"
#include <assert.h>

#pragma warning (disable:4100)

static const char *Id = "$Id: WordNet.cpp,v 1.166 2011/12/10 wn Exp $";

const char *WordNet::wnrelease = "3.0";
const char *WordNet::lexfiles[] = {
    "adj.all", /* 0 */
    "adj.pert", /* 1 */
    "adv.all", /* 2 */
    "noun.Tops", /* 3 */
    "noun.act", /* 4 */
    "noun.animal", /* 5 */
    "noun.artifact", /* 6 */
    "noun.attribute", /* 7 */
    "noun.body", /* 8 */
    "noun.cognition", /* 9 */
    "noun.communication", /* 10 */
    "noun.event", /* 11 */
    "noun.feeling", /* 12 */
    "noun.food", /* 13 */
    "noun.group", /* 14 */
    "noun.location", /* 15 */
    "noun.motive", /* 16 */
    "noun.object", /* 17 */
    "noun.person", /* 18 */
    "noun.phenomenon", /* 19 */
    "noun.plant", /* 20 */
    "noun.possession", /* 21 */
    "noun.process", /* 22 */
    "noun.quantity", /* 23 */
    "noun.relation", /* 24 */
    "noun.shape", /* 25 */
    "noun.state", /* 26 */
    "noun.substance", /* 27 */
    "noun.time", /* 28 */
    "verb.body", /* 29 */
    "verb.change", /* 30 */
    "verb.cognition", /* 31 */
    "verb.communication", /* 32 */
    "verb.competition", /* 33 */
    "verb.consumption", /* 34 */
    "verb.contact", /* 35 */
    "verb.creation", /* 36 */
    "verb.emotion", /* 37 */
    "verb.motion", /* 38 */
    "verb.perception", /* 39 */
    "verb.possession", /* 40 */
    "verb.social", /* 41 */
    "verb.stative", /* 42 */
    "verb.weather", /* 43 */
    "adj.ppl", /* 44 */
};

const char *WordNet::ptrtyp[] = {
    "", /* 0 not used */
    "!", /* 1 ANTPTR */
    "@", /* 2 HYPERPTR */
    "~", /* 3 HYPOPTR */
    "*", /* 4 ENTAILPTR */
    "&", /* 5 SIMPTR */
    "#m", /* 6 ISMEMBERPTR */
    "#s", /* 7 ISSTUFFPTR */
    "#p", /* 8 ISPARTPTR */
    "%m", /* 9 HASMEMBERPTR */
    "%s", /* 10 HASSTUFFPTR */
    "%p", /* 11 HASPARTPTR */
    "%", /* 12 MERONYM */
    "#", /* 13 HOLONYM */
    ">", /* 14 CAUSETO */
    "<", /* 15 PPLPTR */
    "^", /* 16 SEEALSO */
    "\\", /* 17 PERTPTR */
    "=", /* 18 ATTRIBUTE */
    "$", /* 19 VERBGROUP */
    "+", /* 20 NOMINALIZATIONS */
    ";", /* 21 CLASSIFICATION */
    "-", /* 22 CLASS */
    /* additional searches, but not pointers.  */
    "", /* SYNS */
    "", /* FREQ */
    "+", /* FRAMES */
    "", /* COORDS */
    "", /* RELATIVES */
    "", /* HMERONYM */
    "", /* HHOLONYM */
    "", /* WNGREP */
    "", /* OVERVIEW */
    ";c", /* CLASSIF_CATEGORY */
    ";u", /* CLASSIF_USAGE */
    ";r", /* CLASSIF_REGIONAL */
    "-c", /* CLASS_CATEGORY */
    "-u", /* CLASS_USAGE */
    "-r", /* CLASS_REGIONAL */
    "@i", /* INSTANCE */
    "~i", /* INSTANCES */
    NULL,
};

const char *WordNet::partnames[] = {"", "noun", "verb", "adj", "adv", NULL};
const char WordNet::partchars[] = " nvara"; /* add char for satellites to end */
const char *WordNet::adjclass[] = {"", "(p)", "(a)", "(ip)"};
const char *WordNet::frametext[] = {
    "",
    "Something ----s",
    "Somebody ----s",
    "It is ----ing",
    "Something is ----ing PP",
    "Something ----s something Adjective/Noun",
    "Something ----s Adjective/Noun",
    "Somebody ----s Adjective",
    "Somebody ----s something",
    "Somebody ----s somebody",
    "Something ----s somebody",
    "Something ----s something",
    "Something ----s to somebody",
    "Somebody ----s on something",
    "Somebody ----s somebody something",
    "Somebody ----s something to somebody",
    "Somebody ----s something from somebody",
    "Somebody ----s somebody with something",
    "Somebody ----s somebody of something",
    "Somebody ----s something on somebody",
    "Somebody ----s somebody PP",
    "Somebody ----s something PP",
    "Somebody ----s PP",
    "Somebody's (body part) ----s",
    "Somebody ----s somebody to INFINITIVE",
    "Somebody ----s somebody INFINITIVE",
    "Somebody ----s that CLAUSE",
    "Somebody ----s to somebody",
    "Somebody ----s to INFINITIVE",
    "Somebody ----s whether INFINITIVE",
    "Somebody ----s somebody into V-ing something",
    "Somebody ----s something with something",
    "Somebody ----s INFINITIVE",
    "Somebody ----s VERB-ing",
    "It ----s that CLAUSE",
    "Something ----s INFINITIVE",
    ""
};

void (*interface_doevents_func)(void) = NULL;
int (*display_message)(char *) = default_display_message;

/* For adjectives, indicates synset type */

#define DONT_KNOW	0
#define DIRECT_ANT	1	/* direct antonyms (cluster head) */
#define INDIRECT_ANT	2	/* indrect antonyms (similar) */
#define PERTAINYM	3	/* no antonyms or similars (pertainyms) */

/* Flags for printsynset() */

#define ALLWORDS	0	/* print all words */
#define SKIP_ANTS	0	/* skip printing antonyms in printsynset() */
#define PRINT_ANTS	1	/* print antonyms in printsynset() */
#define SKIP_MARKER	0	/* skip printing adjective marker */
#define PRINT_MARKER	1	/* print adjective marker */

/* Trace types used by printspaces() to determine print sytle */

#define TRACEP		1	/* traceptrs */
#define TRACEC		2	/* tracecoords() */
#define TRACEI		3	/* traceinherit() */

#define DEFON 1
#define DEFOFF 0

#ifdef _WINDOWS
#define EXCFILE	"%s\\%s.exc"
#else
#define EXCFILE	"%s/%s.exc"
#endif

static const char *sufx[] = {
    /* Noun suffixes */
    "s", "ses", "xes", "zes", "ches", "shes", "men", "ies",
    /* Verb suffixes */
    "s", "ies", "es", "es", "ed", "ed", "ing", "ing",
    /* Adjective suffixes */
    "er", "est", "er", "est"
};

static const char *addr[] = {
    /* Noun endings */
    "", "s", "x", "z", "ch", "sh", "man", "y",
    /* Verb endings */
    "", "y", "e", "", "e", "", "e", "",
    /* Adjective endings */
    "", "", "e", "e"
};

static int offsets[NUMPARTS] = {0, 0, 8, 16};
static int cnts[NUMPARTS] = {0, 8, 8, 4};
static char msgbuf[256];

#define NUMPREPS	15

static struct {
    const char *str;
    int strlen;
} prepositions[NUMPREPS] = {
    "to", 2,
    "at", 2,
    "of", 2,
    "on", 2,
    "off", 3,
    "in", 2,
    "out", 3,
    "up", 2,
    "down", 4,
    "from", 4,
    "with", 4,
    "into", 4,
    "for", 3,
    "about", 5,
    "between", 7,
};

Synset::Synset() {
    hereiam = 0;
    sstype = DONT_KNOW;
    fnum = 0;
    pos = '\0';
    wcount = 0;
    words = '\0';
    whichword = 0;
    ptrcount = 0;
    ptrtyp = '\0';
    ptroff = '\0';
    ppos = '\0';
    pto = '\0';
    pfrm = '\0';
    fcount = 0;
    frmid = '\0';
    frmto = '\0';
    defn = '\0';
    key = 0;
    nextss = NULL;
    nextform = NULL;
    searchtype = -1;
    ptrlist = NULL;
    headword = NULL;
    headsense = 0;
}

Synset::~Synset() {
    if (pos != NULL)
        delete pos;
    if (words != NULL) {
        for (int i = 0; i < wcount; i++)
            delete words[i];
        delete words;
    }
    if (lexid != NULL)
        delete lexid;
    if (wnsns != NULL)
        delete wnsns;
    if (ptrtyp != NULL)
        delete ptrtyp;
    if (ptroff != NULL)
        delete ptroff;
    if (ppos != NULL)
        delete ppos;
    if (pto != NULL)
        delete pto;
    if (pfrm != NULL)
        delete pfrm;
    if (frmid != NULL)
        delete frmid;
    if (frmto != NULL)
        delete frmto;
    if (defn != NULL)
        delete defn;

    if (nextss != NULL)
        delete nextss;
    if (nextform != NULL)
        delete nextform;
    if (ptrlist != NULL)
        delete ptrlist;
    if (headword != NULL)
        delete headword;
}

static FILE *exc_fps[NUMPARTS + 1];

WordNet::WordNet(void) {
    overflag = 0;

    fnflag = 0; /* if set, print lex filename after sense */
    dflag = 1; /* if set, print definitional glosses */
    saflag = 1; /* if set, print SEE ALSO pointers */
    fileinfoflag = 0; /* if set, print lex file info on synsets */
    frflag = 0; /* if set, print verb frames */
    abortsearch = 0; /* if set, stop search algorithm */
    offsetflag = 0; /* if set, print byte offset of each synset */
    wnsnsflag = 0; /* if set, print WN sense # for each word */

    for (int i = 0; i < 5; i++) {
        datafps[i] = NULL;
        indexfps[i] = NULL;
    }

    sensefp = NULL;
    cntlistfp = NULL;
    keyindexfp = NULL;
    revkeyindexfp = NULL;
    vsentfilefp = NULL;
    vidxfilefp = NULL;

    OpenDB = 0;

    display_message = default_display_message;
    last_bin_search_offset = 0;
}

WordNet::~WordNet(void) {
    closefps();
}

/* Find word in index file and return parsed entry in data structure.
   Input word must be exact match of string in database. */

IndexPtr WordNet::index_lookup(char *word, int dbase) {
    IndexPtr idx = NULL;
    FILE *fp;
    char *line;

    if ((fp = indexfps[dbase]) == NULL) {
        sprintf_s(msgbuf, "WordNet library error: %s indexfile not open\n",
                partnames[dbase]);
        display_message(msgbuf);
        return (NULL);
    }


    if ((line = bin_search(word, fp)) != NULL) {
        idx = parse_index(last_bin_search_offset, dbase, line);
    }

    return (idx);
}

/* This function parses an entry from an index file into an Index data
 * structure. It takes the byte offset and file number, and optionally the
 * line. If the line is NULL, parse_index will get the line from the file.
 * If the line is non-NULL, parse_index won't look at the file, but it still
 * needs the dbase and offset parameters to be set, so it can store them in
 * the Index struct.
 */

IndexPtr WordNet::parse_index(long offset, int dbase, char *line) {
    IndexPtr idx = NULL;
    char *ptrtok;
    int j;

    if (!line)
        line = read_index(offset, indexfps[dbase]);

    idx = (IndexPtr) malloc(sizeof (Index));
    assert(idx);


    /* set offset of entry in index file */
    idx->idxoffset = offset;

    idx->wd = '\0';
    idx->pos = '\0';
    idx->off_cnt = 0;
    idx->tagged_cnt = 0;
    idx->sense_cnt = 0;
    idx->offset = '\0';
    idx->ptruse_cnt = 0;
    idx->ptruse = '\0';
    char *context = NULL;

    /* get the word */
    ptrtok = strtok_s(line, " \n", &context);

    idx->wd = (char *) malloc(strlen(ptrtok) + 1);
    assert(idx->wd);
    strcpy(idx->wd, strlen(ptrtok) + 1, ptrtok);

    /* get the part of speech */
    ptrtok = strtok_s(NULL, " \n", &context);
    idx->pos = (char *) malloc(strlen(ptrtok) + 1);

    assert(idx->pos);
    strcpy(idx->pos, strlen(ptrtok) + 1, ptrtok);

    /* get the collins count */
    ptrtok = strtok_s(NULL, " \n", &context);
    idx->sense_cnt = atoi(ptrtok);

    /* get the number of pointers types */
    ptrtok = strtok_s(NULL, " \n", &context);
    idx->ptruse_cnt = atoi(ptrtok);

    if (idx->ptruse_cnt) {
        idx->ptruse = (int *) malloc(idx->ptruse_cnt * (sizeof (int)));
        assert(idx->ptruse);

        /* get the pointers types */
        for (j = 0; j < idx->ptruse_cnt; j++) {
            ptrtok = strtok_s(NULL, " \n", &context);
            idx->ptruse[j] = getptrtype(ptrtok);
        }
    }

    /* get the number of offsets */
    ptrtok = strtok_s(NULL, " \n", &context);
    idx->off_cnt = atoi(ptrtok);

    /* get the number of senses that are tagged */
    ptrtok = strtok_s(NULL, " \n", &context);
    idx->tagged_cnt = atoi(ptrtok);

    /* make space for the offsets */
    idx->offset = (unsigned long *) malloc(idx->off_cnt * (sizeof (unsigned long)));
    assert(idx->offset);

    /* get the offsets */
    for (j = 0; j < idx->off_cnt; j++) {
        ptrtok = strtok_s(NULL, " \n", &context);
        idx->offset[j] = atol(ptrtok);
    }
    return (idx);
}

/* 'smart' search of index file.  Find word in index file, trying different
   techniques - replace hyphens with underscores, replace underscores with
   hyphens, strip hyphens and underscores, strip periods. */

IndexPtr WordNet::getindex(const char *searchstr, int dbase) {
    int i, j, k;
    char c;
    char ssbuf[WORDBUF];
    char strings[MAX_FORMS][WORDBUF]; /* vector of search strings */
    static IndexPtr offsets[MAX_FORMS];
    static int offset;

    if (searchstr != NULL)
        strcpy(ssbuf, WORDBUF, searchstr);

    /* This works like strrok(): if passed with a non-null string,
       prepare vector of search strings and offsets.  If string
       is null, look at current list of offsets and return next
       one, or NULL if no more alternatives for this word. */

    if (searchstr != NULL) {

        offset = 0;
        strtolower(ssbuf);
        for (i = 0; i < MAX_FORMS; i++) {
            strcpy(strings[i], WORDBUF, ssbuf);
            offsets[i] = 0;
        }

        strsubst(strings[1], '_', '-');
        strsubst(strings[2], '-', '_');

        /* remove all spaces and hyphens from last search string, then
           all periods */
        for (i = j = k = 0; (c = ssbuf[i]) != '\0'; i++) {
            if (c != '_' && c != '-')
                strings[3][j++] = c;
            if (c != '.')
                strings[4][k++] = c;
        }
        strings[3][j] = '\0';
        strings[4][k] = '\0';

        /* Get offset of first entry.  Then eliminate duplicates
           and get offsets of unique strings. */

        if (strings[0][0] != NULL)
            offsets[0] = index_lookup(strings[0], dbase);

        for (i = 1; i < MAX_FORMS; i++)
            if ((strings[i][0]) != NULL && (strcmp(strings[0], strings[i])))
                offsets[i] = index_lookup(strings[i], dbase);
    }

    for (i = offset; i < MAX_FORMS; i++)
        if (offsets[i]) {
            offset = i + 1;
            return (offsets[i]);
        }

    return (NULL);
}

/* Read synset from data file at byte offset passed and return parsed
   entry in data structure. */

SynsetPtr WordNet::read_synset(int dbase, long boffset, char *word) {
    FILE *fp;

    if ((fp = datafps[dbase]) == NULL) {
        sprintf_s(msgbuf, "WordNet library error: %s datafile not open\n",
                partnames[dbase]);
        display_message(msgbuf);
        return (NULL);
    }

    fseek(fp, boffset, 0); /* position file to byte offset requested */

    return (parse_synset(fp, dbase, word)); /* parse synset and return */
}

/* Read synset at current byte offset in file and return parsed entry
   in data structure. */

SynsetPtr WordNet::parse_synset(FILE *fp, int dbase, char *word) {
    char tbuf[SMLINEBUF];
    char *ptrtok;
    char *tmpptr;
    int foundpert = 0;
    char wdnum[3];
    int i;
    SynsetPtr synptr;
    long loc; /* sanity check on file location */
    char *context = NULL;

    loc = ftell(fp);

    if ((tmpptr = fgets(linebuf, LINEBUF, fp)) == NULL)
        return (NULL);

    synptr = new Synset();
    assert(synptr);

    ptrtok = linebuf;

    /* looking at offset */
    ptrtok = strtok_s(linebuf, " \n", &context);
    synptr->hereiam = atol(ptrtok);

    /* sanity check - make sure starting file offset matches first field */
    if (synptr->hereiam != loc) {
        sprintf_s(msgbuf, "WordNet library error: no synset at location %d\n",
                loc);
        display_message(msgbuf);
        free(synptr);
        return (NULL);
    }

    /* looking at FNUM */
    ptrtok = strtok_s(NULL, " \n", &context);
    synptr->fnum = atoi(ptrtok);

    /* looking at POS */
    ptrtok = strtok_s(NULL, " \n", &context);
    synptr->pos = (char *) malloc(strlen(ptrtok) + 1);
    assert(synptr->pos);
    strcpy(synptr->pos, strlen(ptrtok) + 1, ptrtok);
    if (getsstype(synptr->pos) == SATELLITE)
        synptr->sstype = INDIRECT_ANT;

    /* looking at numwords */
    ptrtok = strtok_s(NULL, " \n", &context);
    synptr->wcount = strtol(ptrtok, NULL, 16);

    synptr->words = (char **) malloc(synptr->wcount * sizeof (char *));
    assert(synptr->words);
    synptr->wnsns = (int *) malloc(synptr->wcount * sizeof (int));
    assert(synptr->wnsns);
    synptr->lexid = (int *) malloc(synptr->wcount * sizeof (int));
    assert(synptr->lexid);

    for (i = 0; i < synptr->wcount; i++) {
        ptrtok = strtok_s(NULL, " \n", &context);
        synptr->words[i] = (char *) malloc(strlen(ptrtok) + 1);
        assert(synptr->words[i]);
        strcpy(synptr->words[i], strlen(ptrtok) + 1, ptrtok);

        /* is this the word we're looking for? */

        if (word && !strcmp(word, strtolower(ptrtok)))
            synptr->whichword = i + 1;

        ptrtok = strtok_s(NULL, " \n", &context);
        sscanf(ptrtok, "%x", &synptr->lexid[i]);
    }

    /* get the pointer count */
    ptrtok = strtok_s(NULL, " \n", &context);
    synptr->ptrcount = atoi(ptrtok);

    if (synptr->ptrcount) {

        /* alloc storage for the pointers */
        synptr->ptrtyp = (int *) malloc(synptr->ptrcount * sizeof (int));
        assert(synptr->ptrtyp);
        synptr->ptroff = (long *) malloc(synptr->ptrcount * sizeof (long));
        assert(synptr->ptroff);
        synptr->ppos = (int *) malloc(synptr->ptrcount * sizeof (int));
        assert(synptr->ppos);

        synptr->pto = (int *) malloc(synptr->ptrcount * sizeof (int));
        assert(synptr->pto);

        synptr->pfrm = (int *) malloc(synptr->ptrcount * sizeof (int));
        assert(synptr->pfrm);


        for (i = 0; i < synptr->ptrcount; i++) {
            /* get the pointer type */
            ptrtok = strtok_s(NULL, " \n", &context);
            synptr->ptrtyp[i] = getptrtype(ptrtok);
            /* For adjectives, set the synset type if it has a direct
               antonym */
            if (dbase == ADJ && synptr->sstype == DONT_KNOW) {
                if (synptr->ptrtyp[i] == ANTPTR)
                    synptr->sstype = DIRECT_ANT;
                else if (synptr->ptrtyp[i] == PERTPTR)
                    foundpert = 1;
            }

            /* get the pointer offset */
            ptrtok = strtok_s(NULL, " \n", &context);
            synptr->ptroff[i] = atol(ptrtok);

            /* get the pointer part of speech */
            ptrtok = strtok_s(NULL, " \n", &context);
            synptr->ppos[i] = getpos(ptrtok);

            /* get the lexp to/from restrictions */
            ptrtok = strtok_s(NULL, " \n", &context);

            tmpptr = ptrtok;
            strncpy_s(wdnum, tmpptr, 2);
            wdnum[2] = '\0';
            synptr->pfrm[i] = strtol(wdnum, (char **) NULL, 16);

            tmpptr += 2;
            strncpy_s(wdnum, tmpptr, 2);
            wdnum[2] = '\0';
            synptr->pto[i] = strtol(wdnum, (char **) NULL, 16);
        }
    }

    /* If synset type is still not set, see if it's a pertainym */

    if (dbase == ADJ && synptr->sstype == DONT_KNOW && foundpert == 1)
        synptr->sstype = PERTAINYM;

    /* retireve optional information from verb synset */
    if (dbase == VERB) {
        ptrtok = strtok_s(NULL, " \n", &context);
        synptr->fcount = atoi(ptrtok);

        /* allocate frame storage */

        synptr->frmid = (int *) malloc(synptr->fcount * sizeof (int));
        assert(synptr->frmid);
        synptr->frmto = (int *) malloc(synptr->fcount * sizeof (int));
        assert(synptr->frmto);

        for (i = 0; i < synptr->fcount; i++) {
            /* skip the frame pointer (+) */
            ptrtok = strtok_s(NULL, " \n", &context);

            ptrtok = strtok_s(NULL, " \n", &context);
            synptr->frmid[i] = atoi(ptrtok);

            ptrtok = strtok_s(NULL, " \n", &context);
            synptr->frmto[i] = strtol(ptrtok, NULL, 16);
        }
    }

    /* get the optional definition */

    ptrtok = strtok_s(NULL, " \n", &context);
    if (ptrtok) {
        ptrtok = strtok_s(NULL, " \n", &context);
        sprintf_s(tbuf, "");
        while (ptrtok != NULL) {
            strcat(tbuf, ptrtok);
            ptrtok = strtok_s(NULL, " \n", &context);
            if (ptrtok)
                strcat(tbuf, " ");
        }
        assert((1 + strlen(tbuf)) < sizeof (tbuf));
        synptr->defn = (char *) malloc(strlen(tbuf) + 4);
        assert(synptr->defn);
        sprintf_s(synptr->defn, strlen(tbuf) + 4, "(%s)", tbuf);
    }

    if (keyindexfp) { /* we have unique keys */
        sprintf_s(tmpbuf, TMPBUFSIZE, "%c:%8.8d", partchars[dbase], synptr->hereiam);
        synptr->key = GetKeyForOffset(tmpbuf);
    }

    /* Can't do earlier - calls indexlookup which messes up strtok calls */

    for (i = 0; i < synptr->wcount; i++)
        synptr->wnsns[i] = getsearchsense(synptr, i + 1);

    return (synptr);
}

/* Free a synset linked list allocated by findtheinfo_ds() */

void WordNet::free_syns(SynsetPtr synptr) {
    SynsetPtr cursyn, nextsyn;

    if (synptr) {
        cursyn = synptr;
        while (cursyn) {
            if (cursyn->nextform)
                free_syns(cursyn->nextform);
            nextsyn = cursyn->nextss;
            free_synset(cursyn);
            cursyn = nextsyn;
        }
    }
}

/* Free a synset */

void WordNet::free_synset(SynsetPtr synptr) {
    int i;

    free(synptr->pos);
    for (i = 0; i < synptr->wcount; i++) {
        free(synptr->words[i]);
    }
    free(synptr->words);
    free(synptr->wnsns);
    free(synptr->lexid);
    if (synptr->ptrcount) {
        free(synptr->ptrtyp);
        free(synptr->ptroff);
        free(synptr->ppos);
        free(synptr->pto);
        free(synptr->pfrm);
    }
    if (synptr->fcount) {
        free(synptr->frmid);
        free(synptr->frmto);
    }
    if (synptr->defn)
        free(synptr->defn);
    if (synptr->headword)
        free(synptr->headword);
    if (synptr->ptrlist)
        free_syns(synptr->ptrlist); /* changed from free_synset() */
    free(synptr);
}

/* Free an index structure */

void WordNet::free_index(IndexPtr idx) {
    free(idx->wd);
    free(idx->pos);
    if (idx->ptruse)
        free(idx->ptruse);
    free(idx->offset);
    free(idx);
}

/* Recursive search algorithm to trace a pointer tree */

void WordNet::traceptrs(SynsetPtr synptr, int ptrtyp, int dbase, int depth) {
    int i;
    int extraindent = 0;
    SynsetPtr cursyn;
    char prefix[40], tbuf[20];
    int realptr;

    interface_doevents();
    if (abortsearch)
        return;

    if (ptrtyp < 0) {
        ptrtyp = -ptrtyp;
        extraindent = 2;
    }

    for (i = 0; i < synptr->ptrcount; i++) {
        if ((ptrtyp == HYPERPTR && (synptr->ptrtyp[i] == HYPERPTR ||
                synptr->ptrtyp[i] == INSTANCE)) ||
                (ptrtyp == HYPOPTR && (synptr->ptrtyp[i] == HYPOPTR ||
                synptr->ptrtyp[i] == INSTANCES)) ||
                ((synptr->ptrtyp[i] == ptrtyp) &&
                ((synptr->pfrm[i] == 0) ||
                (synptr->pfrm[i] == synptr->whichword)))) {

            realptr = synptr->ptrtyp[i]; /* deal with INSTANCE */

            if (!prflag) { /* print sense number and synset */
                printsns(synptr, sense + 1);
                prflag = 1;
            }
            printspaces(TRACEP, depth + extraindent);

            switch (realptr) {
                case PERTPTR:
                    if (dbase == ADV)
                        sprintf_s(prefix, "Derived from %s ",
                            partnames[synptr->ppos[i]]);
                    else
                        sprintf_s(prefix, "Pertains to %s ",
                            partnames[synptr->ppos[i]]);
                    break;
                case ANTPTR:
                    if (dbase != ADJ)
                        sprintf_s(prefix, "Antonym of ");
                    break;
                case PPLPTR:
                    sprintf_s(prefix, "Participle of verb ");
                    break;
                case INSTANCE:
                    sprintf_s(prefix, "INSTANCE OF=> ");
                    break;
                case INSTANCES:
                    sprintf_s(prefix, "HAS INSTANCE=> ");
                    break;
                case HASMEMBERPTR:
                    sprintf_s(prefix, "   HAS MEMBER: ");
                    break;
                case HASSTUFFPTR:
                    sprintf_s(prefix, "   HAS SUBSTANCE: ");
                    break;
                case HASPARTPTR:
                    sprintf_s(prefix, "   HAS PART: ");
                    break;
                case ISMEMBERPTR:
                    sprintf_s(prefix, "   MEMBER OF: ");
                    break;
                case ISSTUFFPTR:
                    sprintf_s(prefix, "   SUBSTANCE OF: ");
                    break;
                case ISPARTPTR:
                    sprintf_s(prefix, "   PART OF: ");
                    break;
                default:
                    sprintf_s(prefix, "=> ");
                    break;
            }

            /* Read synset pointed to */
            cursyn = read_synset(synptr->ppos[i], synptr->ptroff[i], "");

            /* For Pertainyms and Participles pointing to a specific
               sense, indicate the sense then retrieve the synset
               pointed to and other info as determined by type.
               Otherwise, just print the synset pointed to. */

            if ((ptrtyp == PERTPTR || ptrtyp == PPLPTR) &&
                    synptr->pto[i] != 0) {
                sprintf_s(tbuf, " (Sense %d)\n",
                        cursyn->wnsns[synptr->pto[i] - 1]);
                printsynset(prefix, cursyn, tbuf, DEFOFF, synptr->pto[i],
                        SKIP_ANTS, PRINT_MARKER);
                if (ptrtyp == PPLPTR) { /* adjective pointing to verb */
                    printsynset("      =>", cursyn, "\n",
                            DEFON, ALLWORDS, PRINT_ANTS, PRINT_MARKER);
                    traceptrs(cursyn, HYPERPTR, getpos(cursyn->pos), 0);
                } else if (dbase == ADV) { /* adverb pointing to adjective */
                    printsynset("      =>", cursyn, "\n", DEFON, ALLWORDS,
                            ((getsstype(cursyn->pos) == SATELLITE)
                            ? SKIP_ANTS : PRINT_ANTS), PRINT_MARKER);
#ifdef FOOP
                    traceptrs(cursyn, HYPERPTR, getpos(cursyn->pos), 0);
#endif
                } else { /* adjective pointing to noun */
                    printsynset("      =>", cursyn, "\n",
                            DEFON, ALLWORDS, PRINT_ANTS, PRINT_MARKER);
                    traceptrs(cursyn, HYPERPTR, getpos(cursyn->pos), 0);
                }
            } else if (ptrtyp == ANTPTR && dbase != ADJ && synptr->pto[i] != 0) {
                sprintf_s(tbuf, " (Sense %d)\n",
                        cursyn->wnsns[synptr->pto[i] - 1]);
                printsynset(prefix, cursyn, tbuf, DEFOFF, synptr->pto[i],
                        SKIP_ANTS, PRINT_MARKER);
                printsynset("      =>", cursyn, "\n", DEFON, ALLWORDS,
                        PRINT_ANTS, PRINT_MARKER);
            } else
                printsynset(prefix, cursyn, "\n", DEFON, ALLWORDS,
                    PRINT_ANTS, PRINT_MARKER);

            /* For HOLONYMS and MERONYMS, keep track of last one
               printed in buffer so results can be truncated later. */

            if (ptrtyp >= ISMEMBERPTR && ptrtyp <= HASPARTPTR)
                lastholomero = strlen(searchbuffer);

            if (depth) {
                depth = depthcheck(depth, cursyn);
                traceptrs(cursyn, ptrtyp, getpos(cursyn->pos), (depth + 1));

                free_synset(cursyn);
            } else
                free_synset(cursyn);
        }
    }
}

void WordNet::tracecoords(SynsetPtr synptr, int ptrtyp, int dbase, int depth) {
    int i;
    SynsetPtr cursyn;

    interface_doevents();
    if (abortsearch)
        return;

    for (i = 0; i < synptr->ptrcount; i++) {
        if ((synptr->ptrtyp[i] == HYPERPTR || synptr->ptrtyp[i] == INSTANCE) &&
                ((synptr->pfrm[i] == 0) ||
                (synptr->pfrm[i] == synptr->whichword))) {

            if (!prflag) {
                printsns(synptr, sense + 1);
                prflag = 1;
            }
            printspaces(TRACEC, depth);

            cursyn = read_synset(synptr->ppos[i], synptr->ptroff[i], "");

            printsynset("-> ", cursyn, "\n", DEFON, ALLWORDS,
                    SKIP_ANTS, PRINT_MARKER);

            traceptrs(cursyn, ptrtyp, getpos(cursyn->pos), depth);

            if (depth) {
                depth = depthcheck(depth, cursyn);
                tracecoords(cursyn, ptrtyp, getpos(cursyn->pos), (depth + 1));
                free_synset(cursyn);
            } else
                free_synset(cursyn);
        }
    }
}

void WordNet::traceclassif(SynsetPtr synptr, int dbase, int search) {
    int i, j, idx;
    SynsetPtr cursyn;
    long int prlist[1024];
    char head[60];
    int svwnsnsflag;

    interface_doevents();
    if (abortsearch)
        return;

    idx = 0;

    for (i = 0; i < synptr->ptrcount; i++) {
        if (((synptr->ptrtyp[i] >= CLASSIF_START) &&
                (synptr->ptrtyp[i] <= CLASSIF_END) && search == CLASSIFICATION) ||

                ((synptr->ptrtyp[i] >= CLASS_START) &&
                (synptr->ptrtyp[i] <= CLASS_END) && search == CLASS)) {

            if (!prflag) {
                printsns(synptr, sense + 1);
                prflag = 1;
            }

            cursyn = read_synset(synptr->ppos[i], synptr->ptroff[i], "");

            for (j = 0; j < idx; j++) {
                if (synptr->ptroff[i] == prlist[j]) {
                    break;
                }
            }

            if (j == idx) {
                prlist[idx++] = synptr->ptroff[i];
                printspaces(TRACEP, 0);

                if (synptr->ptrtyp[i] == CLASSIF_CATEGORY)
                    strcpy(head, "TOPIC->(");
                else if (synptr->ptrtyp[i] == CLASSIF_USAGE)
                    strcpy(head, "USAGE->(");
                else if (synptr->ptrtyp[i] == CLASSIF_REGIONAL)
                    strcpy(head, "REGION->(");
                else if (synptr->ptrtyp[i] == CLASS_CATEGORY)
                    strcpy(head, "TOPIC_TERM->(");
                else if (synptr->ptrtyp[i] == CLASS_USAGE)
                    strcpy(head, "USAGE_TERM->(");
                else if (synptr->ptrtyp[i] == CLASS_REGIONAL)
                    strcpy(head, "REGION_TERM->(");

                strcat(head, partnames[synptr->ppos[i]]);
                strcat(head, ") ");

                svwnsnsflag = wnsnsflag;
                wnsnsflag = 1;

                printsynset(head, cursyn, "\n", DEFOFF, ALLWORDS,
                        SKIP_ANTS, SKIP_MARKER);

                wnsnsflag = svwnsnsflag;
            }

            free_synset(cursyn);
        }
    }
}

void WordNet::tracenomins(SynsetPtr synptr, int dbase) {
    int i, j, idx;
    SynsetPtr cursyn;
    long int prlist[1024];
    char prefix[40], tbuf[20];

    interface_doevents();
    if (abortsearch)
        return;

    idx = 0;

    for (i = 0; i < synptr->ptrcount; i++) {
        if ((synptr->ptrtyp[i] == DERIVATION) &&
                (synptr->pfrm[i] == synptr->whichword)) {

            if (!prflag) {
                printsns(synptr, sense + 1);
                prflag = 1;
            }

            printspaces(TRACEP, 0);

            sprintf_s(prefix, 40, "RELATED TO->(%s) ",
                    partnames[synptr->ppos[i]]);

            cursyn = read_synset(synptr->ppos[i], synptr->ptroff[i], "");

            sprintf_s(tbuf, 20, "#%d\n",
                    cursyn->wnsns[synptr->pto[i] - 1]);
            printsynset(prefix, cursyn, tbuf, DEFOFF, synptr->pto[i],
                    SKIP_ANTS, SKIP_MARKER);

            /* only print synset once, even if more than one link */

            for (j = 0; j < idx; j++) {
#ifdef FOOP
                if (synptr->ptroff[i] == prlist[j]) {
                    break;
                }
#endif
            }

            if (j == idx) {
                prlist[idx++] = synptr->ptroff[i];
                printspaces(TRACEP, 2);
                printsynset("=> ", cursyn, "\n", DEFON, ALLWORDS,
                        SKIP_ANTS, PRINT_MARKER);
            }

            free_synset(cursyn);
        }
    }
}

/* Trace through the hypernym tree and print all MEMBER, STUFF
   and PART info. */

void WordNet::traceinherit(SynsetPtr synptr, int ptrbase, int dbase, int depth) {
    int i;
    SynsetPtr cursyn;

    interface_doevents();
    if (abortsearch)
        return;

    for (i = 0; i < synptr->ptrcount; i++) {
        if ((synptr->ptrtyp[i] == HYPERPTR) &&
                ((synptr->pfrm[i] == 0) ||
                (synptr->pfrm[i] == synptr->whichword))) {

            if (!prflag) {
                printsns(synptr, sense + 1);
                prflag = 1;
            }
            printspaces(TRACEI, depth);

            cursyn = read_synset(synptr->ppos[i], synptr->ptroff[i], "");

            printsynset("=> ", cursyn, "\n", DEFON, ALLWORDS,
                    SKIP_ANTS, PRINT_MARKER);

            traceptrs(cursyn, ptrbase, NOUN, depth);
            traceptrs(cursyn, ptrbase + 1, NOUN, depth);
            traceptrs(cursyn, ptrbase + 2, NOUN, depth);

            if (depth) {
                depth = depthcheck(depth, cursyn);
                traceinherit(cursyn, ptrbase, getpos(cursyn->pos), (depth + 1));
                free_synset(cursyn);
            } else
                free_synset(cursyn);
        }
    }

    /* Truncate search buffer after last holo/meronym printed */
    searchbuffer[lastholomero] = '\0';
}

void WordNet::partsall(SynsetPtr synptr, int ptrtyp) {
    int ptrbase;
    int i, hasptr = 0;

    ptrbase = (ptrtyp == HMERONYM) ? HASMEMBERPTR : ISMEMBERPTR;

    /* First, print out the MEMBER, STUFF, PART info for this synset */

    for (i = 0; i < 3; i++) {
        if (HasPtr(synptr, ptrbase + i)) {
            traceptrs(synptr, ptrbase + i, NOUN, 1);
            hasptr++;
        }
        interface_doevents();
        if (abortsearch)
            return;
    }

    /* Print out MEMBER, STUFF, PART info for hypernyms on
       HMERONYM search only */

    /*    if (hasptr && ptrtyp == HMERONYM) { */
    if (ptrtyp == HMERONYM) {
        lastholomero = strlen(searchbuffer);
        traceinherit(synptr, ptrbase, NOUN, 1);
    }
}

void WordNet::traceadjant(SynsetPtr synptr) {
    SynsetPtr newsynptr;
    int i, j;
    int anttype = DIRECT_ANT;
    SynsetPtr simptr, antptr;
    char similar[] = "        => ";

    /* This search is only applicable for ADJ synsets which have
       either direct or indirect antonyms (not valid for pertainyms). */

    if (synptr->sstype == DIRECT_ANT || synptr->sstype == INDIRECT_ANT) {
        printsns(synptr, sense + 1);
        printbuffer("\n");

        /* if indirect, get cluster head */

        if (synptr->sstype == INDIRECT_ANT) {
            anttype = INDIRECT_ANT;
            i = 0;
            while (synptr->ptrtyp[i] != SIMPTR) i++;
            newsynptr = read_synset(ADJ, synptr->ptroff[i], "");
        } else
            newsynptr = synptr;

        /* find antonyms - if direct, make sure that the antonym
           ptr we're looking at is from this word */

        for (i = 0; i < newsynptr->ptrcount; i++) {

            if (newsynptr->ptrtyp[i] == ANTPTR &&
                    ((anttype == DIRECT_ANT &&
                    newsynptr->pfrm[i] == newsynptr->whichword) ||
                    (anttype == INDIRECT_ANT))) {

                /* read the antonym's synset and print it.  if a
                   direct antonym, print it's satellites. */

                antptr = read_synset(ADJ, newsynptr->ptroff[i], "");

                if (anttype == DIRECT_ANT) {
                    printsynset("", antptr, "\n", DEFON, ALLWORDS,
                            PRINT_ANTS, PRINT_MARKER);
                    for (j = 0; j < antptr->ptrcount; j++) {
                        if (antptr->ptrtyp[j] == SIMPTR) {
                            simptr = read_synset(ADJ, antptr->ptroff[j], "");
                            printsynset(similar, simptr, "\n", DEFON,
                                    ALLWORDS, SKIP_ANTS, PRINT_MARKER);
                            free_synset(simptr);
                        }
                    }
                } else
                    printantsynset(antptr, "\n", anttype, DEFON);

                free_synset(antptr);
            }
        }
        if (newsynptr != synptr)
            free_synset(newsynptr);
    }
}

/* Fetch the given example sentence from the example file and print it out */

void WordNet::getexample(char *offset, char *wd) {
    char *line;
    char sentbuf[512];

    if (vsentfilefp != NULL) {
        if ((line = bin_search(offset, vsentfilefp)) != NULL) {
            while (*line != ' ')
                line++;

            printbuffer("          EX: ");
            sprintf_s(sentbuf, 512, line, wd);
            printbuffer(sentbuf);
        }
    }
}

/* Find the example sentence references in the example sentence index file */

int WordNet::findexample(SynsetPtr synptr) {
    char tbuf[256], *temp, *offset;
    int wdnum;
    int found = 0;
    char *context = NULL;

    if (vidxfilefp != NULL) {
        wdnum = synptr->whichword - 1;

        sprintf_s(tbuf, 256, "%s%%%-1.1d:%-2.2d:%-2.2d::",
                synptr->words[wdnum],
                getpos(synptr->pos),
                synptr->fnum,
                synptr->lexid[wdnum]);

        if ((temp = bin_search(tbuf, vidxfilefp)) != NULL) {

            /* skip over sense key and get sentence numbers */

            temp += strlen(synptr->words[wdnum]) + 11;
            strcpy(tbuf, 256, temp);

            offset = strtok_s(tbuf, " ,\n", &context);

            while (offset) {
                getexample(offset, synptr->words[wdnum]);
                offset = strtok_s(NULL, ",\n", &context);
            }
            found = 1;
        }
    }
    return (found);
}

void WordNet::printframe(SynsetPtr synptr, int prsynset) {
    int i;

    if (prsynset)
        printsns(synptr, sense + 1);

    if (!findexample(synptr)) {
        for (i = 0; i < synptr->fcount; i++) {
            if ((synptr->frmto[i] == synptr->whichword) ||
                    (synptr->frmto[i] == 0)) {
                if (synptr->frmto[i] == synptr->whichword)
                    printbuffer("          => ");
                else
                    printbuffer("          *> ");
                printbuffer(frametext[synptr->frmid[i]]);
                printbuffer("\n");
            }
        }
    }
}

void WordNet::printseealso(SynsetPtr synptr) {
    SynsetPtr cursyn;
    int i, first = 1;
    int svwnsnsflag;
    char firstline_v[] = "          Phrasal Verb-> ";
    char firstline_nar[] = "          Also See-> ";
    char otherlines[] = "; ";
    char *prefix;

    if (getpos(synptr->pos) == VERB)
        prefix = firstline_v;
    else
        prefix = firstline_nar;

    /* Find all SEEALSO pointers from the searchword and print the
       word or synset pointed to. */

    for (i = 0; i < synptr->ptrcount; i++) {
        if ((synptr->ptrtyp[i] == SEEALSOPTR) &&
                ((synptr->pfrm[i] == 0) ||
                (synptr->pfrm[i] == synptr->whichword))) {

            cursyn = read_synset(synptr->ppos[i], synptr->ptroff[i], "");

            svwnsnsflag = wnsnsflag;
            wnsnsflag = 1;
            printsynset(prefix, cursyn, "", DEFOFF,
                    synptr->pto[i] == 0 ? ALLWORDS : synptr->pto[i],
                    SKIP_ANTS, SKIP_MARKER);
            wnsnsflag = svwnsnsflag;

            free_synset(cursyn);

            if (first) {
                prefix = otherlines;
                first = 0;
            }
        }
    }
    if (!first)
        printbuffer("\n");
}

void WordNet::freq_word(IndexPtr index) {
    int familiar = 0;
    int cnt;
    char *a_an[] = {
        "", "a noun", "a verb", "an adjective", "an adverb"
    };
    char *freqcats[] = {
        "extremely rare", "very rare", "rare", "uncommon", "common",
        "familiar", "very familiar", "extremely familiar"
    };

    if (index) {
        cnt = index->sense_cnt;
        if (cnt == 0) familiar = 0;
        if (cnt == 1) familiar = 1;
        if (cnt == 2) familiar = 2;
        if (cnt >= 3 && cnt <= 4) familiar = 3;
        if (cnt >= 5 && cnt <= 8) familiar = 4;
        if (cnt >= 9 && cnt <= 16) familiar = 5;
        if (cnt >= 17 && cnt <= 32) familiar = 6;
        if (cnt > 32) familiar = 7;

        sprintf_s(tmpbuf, TMPBUFSIZE,
                "\n%s used as %s is %s (polysemy count = %d)\n",
                index->wd, a_an[getpos(index->pos)], freqcats[familiar], cnt);
        printbuffer(tmpbuf);
    }
}

void WordNet::wngrep(char *word_passed, int pos) {
    FILE *inputfile;
    char word[256];
    int wordlen, linelen, loc;
    char line[1024];
    int count = 0;

    inputfile = indexfps[pos];
    if (inputfile == NULL) {
        sprintf_s(msgbuf, 256, "WordNet library error: Can't perform compounds "
                "search because %s index file is not open\n", partnames[pos]);
        display_message(msgbuf);
        return;
    }
    rewind(inputfile);

    strcpy(word, 256, word_passed);
    ToLowerCase(word); /* map to lower case for index file search */
    strsubst(word, ' ', '_'); /* replace spaces with underscores */
    wordlen = strlen(word);

    while (fgets(line, 1024, inputfile) != NULL) {
        for (linelen = 0; line[linelen] != ' '; linelen++) {
        }
        if (linelen < wordlen)
            continue;
        line[linelen] = '\0';
        strstr_init(line, word);
        while ((loc = strstr_getnext()) != -1) {
            if (
                    /* at the start of the line */
                    (loc == 0) ||
                    /* at the end of the line */
                    ((linelen - wordlen) == loc) ||
                    /* as a word in the middle of the line */
                    (((line[loc - 1] == '-') || (line[loc - 1] == '_')) &&
                    ((line[loc + wordlen] == '-') || (line[loc + wordlen] == '_')))
                    ) {
                strsubst(line, '_', ' ');
                sprintf_s(tmpbuf, TMPBUFSIZE, "%s\n", line);
                printbuffer(tmpbuf);
                break;
            }
        }
        if (count++ % 2000 == 0) {
            interface_doevents();
            if (abortsearch) break;
        }
    }
}

/* Stucture to keep track of 'relative groups'.  All senses in a relative
   group are displayed together at end of search.  Transitivity is
   supported, so if either of a new set of related senses is already
   in a 'relative group', the other sense is added to that group as well. */

struct relgrp {
    int senses[MAXSENSE];
    struct relgrp *next;
};
static struct relgrp *rellist;

static struct relgrp *mkrellist(void);

/* Simple hash function */
#define HASHTABSIZE	1223	/* Prime number. Must be > 2*MAXTOPS */
#define hash(n) ((n) % HASHTABSIZE)

/* Find relative groups for all senses of target word in given part
   of speech. */

void WordNet::relatives(IndexPtr idx, int dbase) {
    rellist = NULL;

    switch (dbase) {

        case VERB:
            findverbgroups(idx);
            interface_doevents();
            if (abortsearch)
                break;
            printrelatives(idx, VERB);
            break;
        default:
            break;
    }

    free_rellist();
}

void WordNet::findverbgroups(IndexPtr idx) {
    int i, j, k;
    SynsetPtr synset;

    assert(idx);

    /* Read all senses */

    for (i = 0; i < idx->off_cnt; i++) {

        synset = read_synset(VERB, idx->offset[i], idx->wd);

        /* Look for VERBGROUP ptr(s) for this sense.  If found,
           create group for senses, or add to existing group. */

        for (j = 0; j < synset->ptrcount; j++) {
            if (synset->ptrtyp[j] == VERBGROUP) {
                /* Need to find sense number for ptr offset */
                for (k = 0; k < idx->off_cnt; k++) {
                    if ((unsigned long) synset->ptroff[j] == idx->offset[k]) {
                        add_relatives(VERB, idx, i, k);
                        break;
                    }
                }
            }
        }
        free_synset(synset);
    }
}

void WordNet::add_relatives(int pos, IndexPtr idx, int rel1, int rel2) {
    int i;
    struct relgrp *rel, *last = NULL, *r;

    /* If either of the new relatives are already in a relative group,
       then add the other to the existing group (transitivity).
       Otherwise create a new group and add these 2 senses to it. */

    for (rel = rellist; rel; rel = rel->next) {
        if (rel->senses[rel1] == 1 || rel->senses[rel2] == 1) {
            rel->senses[rel1] = rel->senses[rel2] = 1;

            /* If part of another relative group, merge the groups */
            for (r = rellist; r; r = r->next) {
                if (r != rel &&
                        (r->senses[rel1] == 1 || r->senses[rel2] == 1)) {
                    for (i = 0; i < MAXSENSE; i++)
                        rel->senses[i] |= r->senses[i];
                }
            }
            return;
        }
        last = rel;
    }
    rel = mkrellist();
    rel->senses[rel1] = rel->senses[rel2] = 1;
    if (rellist == NULL)
        rellist = rel;
    else
        last->next = rel;
}

struct relgrp *WordNet::mkrellist(void) {
    struct relgrp *rel;
    int i;

    rel = (struct relgrp *) malloc(sizeof (struct relgrp));
    assert(rel);
    for (i = 0; i < MAXSENSE; i++)
        rel->senses[i] = 0;
    rel->next = NULL;
    return (rel);
}

void WordNet::free_rellist(void) {
    struct relgrp *rel, *next;

    rel = rellist;
    while (rel) {
        next = rel->next;
        free(rel);
        rel = next;
    }
}

void WordNet::printrelatives(IndexPtr idx, int dbase) {
    SynsetPtr synptr;
    struct relgrp *rel;
    int i, flag;
    int outsenses[MAXSENSE];

    for (i = 0; i < idx->off_cnt; i++)
        outsenses[i] = 0;
    prflag = 1;

    for (rel = rellist; rel; rel = rel->next) {
        flag = 0;
        for (i = 0; i < idx->off_cnt; i++) {
            if (rel->senses[i] && !outsenses[i]) {
                flag = 1;
                synptr = read_synset(dbase, idx->offset[i], "");
                printsns(synptr, i + 1);
                traceptrs(synptr, HYPERPTR, dbase, 0);
                outsenses[i] = 1;
                free_synset(synptr);
            }
        }
        if (flag)
            printbuffer("--------------\n");
    }

    for (i = 0; i < idx->off_cnt; i++) {
        if (!outsenses[i]) {
            synptr = read_synset(dbase, idx->offset[i], "");
            printsns(synptr, i + 1);
            traceptrs(synptr, HYPERPTR, dbase, 0);
            printbuffer("--------------\n");
            free_synset(synptr);
        }
    }
}

/*
  Search code interfaces to WordNet database

  findtheinfo() - print search results and return ptr to output buffer
  findtheinfo_ds() - return search results in linked list data structrure
 */

char *WordNet::findtheinfo(char *searchstr, int dbase, int ptrtyp, int whichsense) {
    SynsetPtr cursyn;
    IndexPtr idx = NULL;
    int depth = 0;
    int i, offsetcnt;
    char *bufstart;
    unsigned long offsets[MAXSENSE];
    int skipit;

    /* Initializations -
       clear output buffer, search results structure, flags */

    searchbuffer[0] = '\0';

    wnresults.numforms = wnresults.printcnt = 0;
    wnresults.searchbuf = searchbuffer;
    wnresults.searchds = NULL;

    abortsearch = overflag = 0;
    for (i = 0; i < MAXSENSE; i++)
        offsets[i] = 0;

    switch (ptrtyp) {
        case OVERVIEW:
            WNOverview(searchstr, dbase);
            break;
        case FREQ:
            while ((idx = getindex(searchstr, dbase)) != NULL) {
                searchstr = NULL;
                wnresults.SenseCount[wnresults.numforms] = idx->off_cnt;
                freq_word(idx);
                free_index(idx);
                wnresults.numforms++;
            }
            break;
        case WNGREP:
            wngrep(searchstr, dbase);
            break;
        case RELATIVES:
        case VERBGROUP:
            while ((idx = getindex(searchstr, dbase)) != NULL) {
                searchstr = NULL;
                wnresults.SenseCount[wnresults.numforms] = idx->off_cnt;
                relatives(idx, dbase);
                free_index(idx);
                wnresults.numforms++;
            }
            break;
        default:

            /* If negative search type, set flag for recursive search */
            if (ptrtyp < 0) {
                ptrtyp = -ptrtyp;
                depth = 1;
            }
            bufstart = searchbuffer;
            offsetcnt = 0;

            /* look at all spellings of word */

            while ((idx = getindex(searchstr, dbase)) != NULL) {

                searchstr = NULL; /* clear out for next call to getindex() */
                wnresults.SenseCount[wnresults.numforms] = idx->off_cnt;
                wnresults.OutSenseCount[wnresults.numforms] = 0;

                /* Print extra sense msgs if looking at all senses */
                if (whichsense == ALLSENSES)
                    printbuffer(
                        "                                                                         \n");

                /* Go through all of the searchword's senses in the
                   database and perform the search requested. */

                for (sense = 0; sense < idx->off_cnt; sense++) {

                    if (whichsense == ALLSENSES || whichsense == sense + 1) {
                        prflag = 0;

                        /* Determine if this synset has already been done
                           with a different spelling. If so, skip it. */
                        for (i = 0, skipit = 0; i < offsetcnt && !skipit; i++) {
                            if (offsets[i] == idx->offset[sense])
                                skipit = 1;
                        }
                        if (skipit != 1) {
                            offsets[offsetcnt++] = idx->offset[sense];
                            cursyn = read_synset(dbase, idx->offset[sense], idx->wd);
                            switch (ptrtyp) {
                                case ANTPTR:
                                    if (dbase == ADJ)
                                        traceadjant(cursyn);
                                    else
                                        traceptrs(cursyn, ANTPTR, dbase, depth);
                                    break;

                                case COORDS:
                                    tracecoords(cursyn, HYPOPTR, dbase, depth);
                                    break;

                                case FRAMES:
                                    printframe(cursyn, 1);
                                    break;

                                case MERONYM:
                                    traceptrs(cursyn, HASMEMBERPTR, dbase, depth);
                                    traceptrs(cursyn, HASSTUFFPTR, dbase, depth);
                                    traceptrs(cursyn, HASPARTPTR, dbase, depth);
                                    break;

                                case HOLONYM:
                                    traceptrs(cursyn, ISMEMBERPTR, dbase, depth);
                                    traceptrs(cursyn, ISSTUFFPTR, dbase, depth);
                                    traceptrs(cursyn, ISPARTPTR, dbase, depth);
                                    break;

                                case HMERONYM:
                                    partsall(cursyn, HMERONYM);
                                    break;

                                case HHOLONYM:
                                    partsall(cursyn, HHOLONYM);
                                    break;

                                case SEEALSOPTR:
                                    printseealso(cursyn);
                                    break;

#ifdef FOOP
                                case PPLPTR:
                                    traceptrs(cursyn, ptrtyp, dbase, depth);
                                    traceptrs(cursyn, PPLPTR, dbase, depth);
                                    break;
#endif

                                case SIMPTR:
                                case SYNS:
                                case HYPERPTR:
                                    printsns(cursyn, sense + 1);
                                    prflag = 1;

                                    traceptrs(cursyn, ptrtyp, dbase, depth);

                                    if (dbase == ADJ) {
                                        /*			    	traceptrs(cursyn, PERTPTR, dbase, depth); */
                                        traceptrs(cursyn, PPLPTR, dbase, depth);
                                    } else if (dbase == ADV) {
                                        /*			    	traceptrs(cursyn, PERTPTR, dbase, depth);*/
                                    }

                                    if (saflag) /* print SEE ALSO pointers */
                                        printseealso(cursyn);

                                    if (dbase == VERB && frflag)
                                        printframe(cursyn, 0);
                                    break;

                                case PERTPTR:
                                    printsns(cursyn, sense + 1);
                                    prflag = 1;

                                    traceptrs(cursyn, PERTPTR, dbase, depth);
                                    break;

                                case DERIVATION:
                                    tracenomins(cursyn, dbase);
                                    break;

                                case CLASSIFICATION:
                                case CLASS:
                                    traceclassif(cursyn, dbase, ptrtyp);
                                    break;

                                default:
                                    traceptrs(cursyn, ptrtyp, dbase, depth);
                                    break;

                            } /* end switch */

                            free_synset(cursyn);

                        } /* end if (skipit) */

                    } /* end if (whichsense) */

                    if (skipit != 1) {
                        interface_doevents();
                        if ((whichsense == sense + 1) || abortsearch || overflag)
                            break; /* break out of loop - we're done */
                    }

                } /* end for (sense) */

                /* Done with an index entry - patch in number of senses output */

                if (whichsense == ALLSENSES) {
                    i = wnresults.OutSenseCount[wnresults.numforms];
                    if (i == idx->off_cnt && i == 1)
                        sprintf_s(tmpbuf, TMPBUFSIZE, "\n1 sense of %s", idx->wd);
                    else if (i == idx->off_cnt)
                        sprintf_s(tmpbuf, TMPBUFSIZE, "\n%d senses of %s", i, idx->wd);
                    else if (i > 0) /* printed some senses */
                        sprintf_s(tmpbuf, TMPBUFSIZE, "\n%d of %d senses of %s",
                            i, idx->off_cnt, idx->wd);

                    /* Find starting offset in searchbuffer for this index
                       entry and patch string in.  Then update bufstart
                       to end of searchbuffer for start of next index entry. */

                    if (i > 0) {
                        if (wnresults.numforms > 0) {
                            bufstart[0] = '\n';
                            bufstart++;
                        }
                        strncpy_s(bufstart, SEARCHBUF - 1, tmpbuf, strlen(tmpbuf));
                        bufstart = searchbuffer + strlen(searchbuffer);
                    }
                }

                free_index(idx);

                interface_doevents();
                if (overflag || abortsearch)
                    break; /* break out of while (idx) loop */

                wnresults.numforms++;

            } /* end while (idx) */

    } /* end switch */

    interface_doevents();
    if (abortsearch)
        printbuffer("\nSearch Interrupted...\n");
    else if (overflag)
        sprintf_s(searchbuffer, SEARCHBUF,
            "Search too large.  Narrow search and try again...\n");

    /* replace underscores with spaces before returning */

    return (strsubst(searchbuffer, '_', ' '));
}

SynsetPtr WordNet::findtheinfo_ds(char *searchstr, int dbase, int ptrtyp, int whichsense) {
    IndexPtr idx;
    SynsetPtr cursyn;
    SynsetPtr synlist = NULL, lastsyn = NULL;
    int depth = 0;
    int newsense = 0;
    char ssbuf[256];

    strcpy(ssbuf, 256, searchstr);

    wnresults.numforms = 0;
    wnresults.printcnt = 0;

    while ((idx = getindex(ssbuf, dbase)) != NULL) {

        ssbuf[0] = '\0';
        // searchstr = NULL;	/* clear out for next call */
        newsense = 1;

        if (ptrtyp < 0) {
            ptrtyp = -ptrtyp;
            depth = 1;
        }

        wnresults.SenseCount[wnresults.numforms] = idx->off_cnt;
        wnresults.OutSenseCount[wnresults.numforms] = 0;
        wnresults.searchbuf = NULL;
        wnresults.searchds = NULL;

        /* Go through all of the searchword's senses in the
           database and perform the search requested. */

        for (sense = 0; sense < idx->off_cnt; sense++) {

            if (whichsense == ALLSENSES || whichsense == sense + 1) {
                cursyn = read_synset(dbase, idx->offset[sense], idx->wd);
                if (lastsyn) {
                    if (newsense)
                        lastsyn->nextform = cursyn;
                    else
                        lastsyn->nextss = cursyn;
                }
                if (!synlist)
                    synlist = cursyn;
                newsense = 0;

                cursyn->searchtype = ptrtyp;
                cursyn->ptrlist = traceptrs_ds(cursyn, ptrtyp,
                        getpos(cursyn->pos),
                        depth);

                lastsyn = cursyn;

                if (whichsense == sense + 1)
                    break;
            }
        }
        free_index(idx);
        wnresults.numforms++;

        if (ptrtyp == COORDS) { /* clean up by removing hypernym */
            lastsyn = synlist->ptrlist;
            synlist->ptrlist = lastsyn->ptrlist;
            free_synset(lastsyn);
        }
    }
    wnresults.searchds = synlist;
    return (synlist);
}

/* Recursive search algorithm to trace a pointer tree and return results
  in linked list of data structures. */

SynsetPtr WordNet::traceptrs_ds(SynsetPtr synptr, int ptrtyp, int dbase, int depth) {
    int i;
    SynsetPtr cursyn, synlist = NULL, lastsyn = NULL;
    int tstptrtyp, docoords;

    /* If synset is a satellite, find the head word of its
       head synset and the head word's sense number. */

    if (getsstype(synptr->pos) == SATELLITE) {
        for (i = 0; i < synptr->ptrcount; i++)
            if (synptr->ptrtyp[i] == SIMPTR) {
                cursyn = read_synset(synptr->ppos[i],
                        synptr->ptroff[i],
                        "");
                synptr->headword = (char *) malloc(strlen(cursyn->words[0]) + 1);
                assert(synptr->headword);
                strcpy(synptr->headword, strlen(cursyn->words[0]) + 1, cursyn->words[0]);
                synptr->headsense = (short) cursyn->lexid[0];
                free_synset(cursyn);
                break;
            }
    }

    if (ptrtyp == COORDS) {
        tstptrtyp = HYPERPTR;
        docoords = 1;
    } else {
        tstptrtyp = ptrtyp;
        docoords = 0;
    }

    for (i = 0; i < synptr->ptrcount; i++) {
        if ((synptr->ptrtyp[i] == tstptrtyp) &&
                ((synptr->pfrm[i] == 0) ||
                (synptr->pfrm[i] == synptr->whichword))) {

            cursyn = read_synset(synptr->ppos[i], synptr->ptroff[i], "");
            cursyn->searchtype = ptrtyp;

            if (lastsyn)
                lastsyn->nextss = cursyn;
            if (!synlist)
                synlist = cursyn;
            lastsyn = cursyn;

            if (depth) {
                depth = depthcheck(depth, cursyn);
                cursyn->ptrlist = traceptrs_ds(cursyn, ptrtyp,
                        getpos(cursyn->pos),
                        (depth + 1));
            } else if (docoords) {
                cursyn->ptrlist = traceptrs_ds(cursyn, HYPOPTR, NOUN, 0);
            }
        }
    }
    return (synlist);
}

void WordNet::WNOverview(char *searchstr, int pos) {
    SynsetPtr cursyn;
    IndexPtr idx = NULL;
    char *cpstring = searchstr, *bufstart;
    int sense, i, offsetcnt;
    int svdflag, skipit;
    unsigned long offsets[MAXSENSE];

    cpstring = searchstr;
    bufstart = searchbuffer;
    for (i = 0; i < MAXSENSE; i++)
        offsets[i] = 0;
    offsetcnt = 0;

    while ((idx = getindex(cpstring, pos)) != NULL) {

        cpstring = NULL; /* clear for next call to getindex() */
        wnresults.SenseCount[wnresults.numforms++] = idx->off_cnt;
        wnresults.OutSenseCount[wnresults.numforms] = 0;

        printbuffer(
                "                                                                                                   \n");

        /* Print synset for each sense.  If requested, precede
           synset with synset offset and/or lexical file information.*/

        for (sense = 0; sense < idx->off_cnt; sense++) {

            for (i = 0, skipit = 0; i < offsetcnt && !skipit; i++)
                if (offsets[i] == idx->offset[sense])
                    skipit = 1;

            if (!skipit) {
                offsets[offsetcnt++] = idx->offset[sense];
                cursyn = read_synset(pos, idx->offset[sense], idx->wd);
                if (idx->tagged_cnt != -1 &&
                        ((sense + 1) <= idx->tagged_cnt)) {
                    sprintf_s(tmpbuf, TMPBUFSIZE, "%d. (%d) ",
                            sense + 1, GetTagcnt(idx, sense + 1));
                } else {
                    sprintf_s(tmpbuf, TMPBUFSIZE, "%d. ", sense + 1);
                }

                svdflag = dflag;
                dflag = 1;
                printsynset(tmpbuf, cursyn, "\n", DEFON, ALLWORDS,
                        SKIP_ANTS, SKIP_MARKER);
                dflag = svdflag;
                wnresults.OutSenseCount[wnresults.numforms]++;
                wnresults.printcnt++;

                free_synset(cursyn);
            }
        }

        /* Print sense summary message */

        i = wnresults.OutSenseCount[wnresults.numforms];

        if (i > 0) {
            if (i == 1)
                sprintf_s(tmpbuf, TMPBUFSIZE, "\nThe %s %s has 1 sense",
                    partnames[pos], idx->wd);
            else
                sprintf_s(tmpbuf, TMPBUFSIZE, "\nThe %s %s has %d senses",
                    partnames[pos], idx->wd, i);
            if (idx->tagged_cnt > 0)
                sprintf_s(tmpbuf + strlen(tmpbuf), TMPBUFSIZE - strlen(tmpbuf),
                    " (first %d from tagged texts)\n", idx->tagged_cnt);
            else if (idx->tagged_cnt == 0)
                sprintf_s(tmpbuf + strlen(tmpbuf), TMPBUFSIZE - strlen(tmpbuf),
                    " (no senses from tagged texts)\n");

            strncpy_s(bufstart, SEARCHBUF, tmpbuf, strlen(tmpbuf));
            bufstart = searchbuffer + strlen(searchbuffer);
        } else
            bufstart[0] = '\0';

        wnresults.numforms++;
        free_index(idx);
    }
}

/* Do requested search on synset passed, returning output in buffer. */

char *WordNet::do_trace(SynsetPtr synptr, int ptrtyp, int dbase, int depth) {
    searchbuffer[0] = '\0'; /* clear output buffer */
    traceptrs(synptr, ptrtyp, dbase, depth);
    return (searchbuffer);
}

/* Set bit for each search type that is valid for the search word
   passed and return bit mask. */

unsigned int WordNet::is_defined(const char *searchstr, int dbase) {
    IndexPtr index;
    char ssbuf[256];
    char *bufptr = ssbuf;
    int i;
    unsigned long retval = 0;

    strcpy(ssbuf, 256, searchstr);

    wnresults.numforms = wnresults.printcnt = 0;
    wnresults.searchbuf = NULL;
    wnresults.searchds = NULL;

    while ((index = getindex(bufptr, dbase)) != NULL) {
        bufptr = NULL; /* clear out for next getindex() call */

        wnresults.SenseCount[wnresults.numforms] = index->off_cnt;

        /* set bits that must be true for all words */

        retval |= bit(SIMPTR) | bit(FREQ) | bit(SYNS) |
                bit(WNGREP) | bit(OVERVIEW);

        /* go through list of pointer characters and set appropriate bits */

        for (i = 0; i < index->ptruse_cnt; i++) {

            if (index->ptruse[i] <= LASTTYPE) {
                retval |= bit(index->ptruse[i]);
            } else if (index->ptruse[i] == INSTANCE) {
                retval |= bit(HYPERPTR);
            } else if (index->ptruse[i] == INSTANCES) {
                retval |= bit(HYPOPTR);
            }

            if (index->ptruse[i] == SIMPTR) {
                retval |= bit(ANTPTR);
            }
#ifdef FOOP

            if (index->ptruse[i] >= CLASSIF_START &&
                    index->ptruse[i] <= CLASSIF_END) {
                retval |= bit(CLASSIFICATION);
            }


            if (index->ptruse[i] >= CLASS_START &&
                    index->ptruse[i] <= CLASS_END) {
                retval |= bit(CLASS);
            }
#endif

            if (index->ptruse[i] >= ISMEMBERPTR &&
                    index->ptruse[i] <= ISPARTPTR)
                retval |= bit(HOLONYM);
            else if (index->ptruse[i] >= HASMEMBERPTR &&
                    index->ptruse[i] <= HASPARTPTR)
                retval |= bit(MERONYM);

        }

        if (dbase == NOUN) {

            /* check for inherited holonyms and meronyms */

            if (HasHoloMero(index, HMERONYM))
                retval |= bit(HMERONYM);
            if (HasHoloMero(index, HHOLONYM))
                retval |= bit(HHOLONYM);

            /* if synset has hypernyms, enable coordinate search */

            if (retval & bit(HYPERPTR))
                retval |= bit(COORDS);
        } else if (dbase == VERB) {

            /* if synset has hypernyms, enable coordinate search */
            if (retval & bit(HYPERPTR))
                retval |= bit(COORDS);

            /* enable grouping of related synsets and verb frames */

            retval |= bit(RELATIVES) | bit(FRAMES);
        }

        free_index(index);
        wnresults.numforms++;
    }
    return (retval);
}

/* Determine if any of the synsets that this word is in have inherited
   meronyms or holonyms. */

int WordNet::HasHoloMero(IndexPtr index, int ptrtyp) {
    int i, j;
    SynsetPtr synset, psynset;
    int found = 0;
    int ptrbase;

    ptrbase = (ptrtyp == HMERONYM) ? HASMEMBERPTR : ISMEMBERPTR;

    for (i = 0; i < index->off_cnt; i++) {
        synset = read_synset(NOUN, index->offset[i], "");
        for (j = 0; j < synset->ptrcount; j++) {
            if (synset->ptrtyp[j] == HYPERPTR) {
                psynset = read_synset(NOUN, synset->ptroff[j], "");
                found += HasPtr(psynset, ptrbase);
                found += HasPtr(psynset, ptrbase + 1);
                found += HasPtr(psynset, ptrbase + 2);

                free_synset(psynset);
            }
        }
        free_synset(synset);
    }
    return (found);
}

int WordNet::HasPtr(SynsetPtr synptr, int ptrtyp) {
    int i;

    for (i = 0; i < synptr->ptrcount; i++) {
        if (synptr->ptrtyp[i] == ptrtyp) {
            return (1);
        }
    }
    return (0);
}

/* Set bit for each POS that search word is in.  0 returned if
   word is not in WordNet. */

unsigned int WordNet::in_wn(char *word, int pos) {
    int i;
    unsigned int retval = 0;

    if (pos == ALL_POS) {
        for (i = 1; i < NUMPARTS + 1; i++)
            if (indexfps[i] != NULL && bin_search(word, indexfps[i]) != NULL)
                retval |= bit(i);
    } else if (indexfps[pos] != NULL && bin_search(word, indexfps[pos]) != NULL)
        retval |= bit(pos);
    return (retval);
}

int WordNet::depthcheck(int depth, SynsetPtr synptr) {
    if (depth >= MAXDEPTH) {
        sprintf_s(msgbuf, 256,
                "WordNet library error: Error Cycle detected\n   %s\n",
                synptr->words[0]);
        display_message(msgbuf);
        depth = -1; /* reset to get one more trace then quit */
    }
    return (depth);
}

/* Strip off () enclosed comments from a word */

char *WordNet::deadjify(char *word) {
    char *y;

    adj_marker = UNKNOWN_MARKER; /* default if not adj or unknown */

    y = word;
    while (*y) {
        if (*y == '(') {
            if (!strncmp(y, "(a)", 3))
                adj_marker = ATTRIBUTIVE;
            else if (!strncmp(y, "(ip)", 4))
                adj_marker = IMMED_POSTNOMINAL;
            else if (!strncmp(y, "(p)", 3))
                adj_marker = PREDICATIVE;
            *y = '\0';
        } else
            y++;
    }
    return (word);
}

int WordNet::getsearchsense(SynsetPtr synptr, int whichword) {
    IndexPtr idx;
    int i;

    strcpy(wdbuf, WORDBUF, synptr->words[whichword - 1]);
    strsubst(wdbuf, ' ', '_');
    strtolower(wdbuf);

    if ((idx = index_lookup(wdbuf, getpos(synptr->pos))) != NULL) {
        for (i = 0; i < idx->off_cnt; i++)
            if (idx->offset[i] == (unsigned long) synptr->hereiam) {
                free_index(idx);
                return (i + 1);
            }
        free_index(idx);
    }
    return (0);
}

void WordNet::printsynset(char *head, SynsetPtr synptr, char *tail, int definition, int wdnum, int antflag, int markerflag) {
    int i, wdcnt;
    char tbuf[SMLINEBUF];

    tbuf[0] = '\0'; /* clear working buffer */

    strcat(tbuf, SMLINEBUF, head); /* print head */

    /* Precede synset with additional information as indiecated
       by flags */

    if (offsetflag) /* print synset offset */
        sprintf_s(tbuf + strlen(tbuf), SMLINEBUF - strlen(tbuf), "{%8.8d} ", synptr->hereiam);
    if (fileinfoflag) { /* print lexicographer file information */
        sprintf_s(tbuf + strlen(tbuf), SMLINEBUF - strlen(tbuf), "<%s> ", lexfiles[synptr->fnum]);
        prlexid = 1; /* print lexicographer id after word */
    } else
        prlexid = 0;

    if (wdnum) /* print only specific word asked for */
        catword(tbuf, SMLINEBUF, synptr, wdnum - 1, markerflag, antflag);
    else /* print all words in synset */
        for (i = 0, wdcnt = synptr->wcount; i < wdcnt; i++) {
            catword(tbuf, SMLINEBUF, synptr, i, markerflag, antflag);
            if (i < wdcnt - 1)
                strcat(tbuf, SMLINEBUF, ", ");
        }

    if (definition && dflag && synptr->defn) {
        strcat(tbuf, SMLINEBUF, " -- ");
        strcat(tbuf, SMLINEBUF, synptr->defn);
    }

    strcat(tbuf, SMLINEBUF, tail);
    printbuffer(tbuf);
}

void WordNet::printantsynset(SynsetPtr synptr, char *tail, int anttype, int definition) {
    int i, wdcnt;
    char tbuf[SMLINEBUF];
    char *str;
    int first = 1;

    tbuf[0] = '\0';

    if (offsetflag)
        sprintf_s(tbuf, SMLINEBUF, "{%8.8d} ", synptr->hereiam);
    if (fileinfoflag) {
        sprintf_s(tbuf + strlen(tbuf), SMLINEBUF - strlen(tbuf), "<%s> ", lexfiles[synptr->fnum]);
        prlexid = 1;
    } else
        prlexid = 0;

    /* print anotnyms from cluster head (of indirect ant) */

    strcat(tbuf, SMLINEBUF, "INDIRECT (VIA ");
    for (i = 0, wdcnt = synptr->wcount; i < wdcnt; i++) {
        if (first) {
            str = printant(ADJ, synptr, i + 1, "%s", ", ");
            first = 0;
        } else
            str = printant(ADJ, synptr, i + 1, ", %s", ", ");
        if (*str)
            strcat(tbuf, SMLINEBUF, str);
    }
    strcat(tbuf, SMLINEBUF, ") -> ");

    /* now print synonyms from cluster head (of indirect ant) */

    for (i = 0, wdcnt = synptr->wcount; i < wdcnt; i++) {
        catword(tbuf, SMLINEBUF, synptr, i, SKIP_MARKER, SKIP_ANTS);
        if (i < wdcnt - 1)
            strcat(tbuf, SMLINEBUF, ", ");
    }

    if (dflag && synptr->defn && definition) {
        strcat(tbuf, SMLINEBUF, " -- ");
        strcat(tbuf, SMLINEBUF, synptr->defn);
    }

    strcat(tbuf, SMLINEBUF, tail);
    printbuffer(tbuf);
}

void WordNet::catword(char *buf, int bufsize, SynsetPtr synptr, int wdnum, int adjmarker, int antflag) {
    char vs[] = " (vs. %s)";
    char *markers[] = {
        "", /* UNKNOWN_MARKER */
        "(predicate)", /* PREDICATIVE */
        "(prenominal)", /* ATTRIBUTIVE */
        "(postnominal)", /* IMMED_POSTNOMINAL */
    };

    /* Copy the word (since deadjify() changes original string),
       deadjify() the copy and append to buffer */

    strcpy(wdbuf, WORDBUF, synptr->words[wdnum]);
    strcat(buf, bufsize, deadjify(wdbuf));

    /* Print additional lexicographer information and WordNet sense
       number as indicated by flags */

    if (prlexid && (synptr->lexid[wdnum] != 0))
        sprintf_s(buf + strlen(buf), bufsize - strlen(buf), "%d", synptr->lexid[wdnum]);
    if (wnsnsflag)
        sprintf_s(buf + strlen(buf), bufsize - strlen(buf), "#%d", synptr->wnsns[wdnum]);

    /* For adjectives, append adjective marker if present, and
       print antonym if flag is passed */

    if (getpos(synptr->pos) == ADJ) {
        if (adjmarker == PRINT_MARKER)
            strcat(buf, bufsize, markers[adj_marker]);
        if (antflag == PRINT_ANTS)
            strcat(buf, bufsize, printant(ADJ, synptr, wdnum + 1, vs, ""));
    }
}

char *WordNet::printant(int dbase, SynsetPtr synptr, int wdnum, char *templ, char *tail) {
    int i, j, wdoff;
    SynsetPtr psynptr;
    char tbuf[WORDBUF];
    static char retbuf[SMLINEBUF];
    int first = 1;

    retbuf[0] = '\0';

    /* Go through all the pointers looking for anotnyms from the word
       indicated by wdnum.  When found, print all the antonym's
       antonym pointers which point back to wdnum. */

    for (i = 0; i < synptr->ptrcount; i++) {
        if (synptr->ptrtyp[i] == ANTPTR && synptr->pfrm[i] == wdnum) {

            psynptr = read_synset(dbase, synptr->ptroff[i], "");

            for (j = 0; j < psynptr->ptrcount; j++) {
                if (psynptr->ptrtyp[j] == ANTPTR &&
                        psynptr->pto[j] == wdnum &&
                        psynptr->ptroff[j] == synptr->hereiam) {

                    wdoff = (psynptr->pfrm[j] ? (psynptr->pfrm[j] - 1) : 0);

                    /* Construct buffer containing formatted antonym,
                       then add it onto end of return buffer */

                    strcpy(wdbuf, WORDBUF, psynptr->words[wdoff]);
                    strcpy(tbuf, WORDBUF, deadjify(wdbuf));

                    /* Print additional lexicographer information and
                       WordNet sense number as indicated by flags */

                    if (prlexid && (psynptr->lexid[wdoff] != 0))
                        sprintf_s(tbuf + strlen(tbuf), SMLINEBUF - strlen(tbuf), "%d",
                            psynptr->lexid[wdoff]);
                    if (wnsnsflag)
                        sprintf_s(tbuf + strlen(tbuf), SMLINEBUF - strlen(tbuf), "#%d",
                            psynptr->wnsns[wdoff]);
                    if (!first)
                        strcat(retbuf, SMLINEBUF, tail);
                    else
                        first = 0;
                    sprintf_s(retbuf + strlen(retbuf), SMLINEBUF - strlen(retbuf), templ, tbuf);
                }
            }
            free_synset(psynptr);
        }
    }
    return (retbuf);
}

void WordNet::printbuffer(char *string) {
    if (overflag)
        return;
    if (strlen(searchbuffer) + strlen(string) >= SEARCHBUF)
        overflag = 1;
    else
        strcat(searchbuffer, SEARCHBUF, string);
}

void WordNet::printsns(SynsetPtr synptr, int sense) {
    printsense(synptr, sense);
    printsynset("", synptr, "\n", DEFON, ALLWORDS, PRINT_ANTS, PRINT_MARKER);
}

void WordNet::printsense(SynsetPtr synptr, int sense) {
    char tbuf[256];

    /* Append lexicographer filename after Sense # if flag is set. */

    if (fnflag)
        sprintf_s(tbuf, 256, "\nSense %d in file \"%s\"\n",
            sense, lexfiles[synptr->fnum]);
    else
        sprintf_s(tbuf, 256, "\nSense %d\n", sense);

    printbuffer(tbuf);

    /* update counters */
    wnresults.OutSenseCount[wnresults.numforms]++;
    wnresults.printcnt++;
}

void WordNet::printspaces(int trace, int depth) {
    int j;

    for (j = 0; j < depth; j++)
        printbuffer("    ");

    switch (trace) {
        case TRACEP: /* traceptrs(), tracenomins() */
            if (depth)
                printbuffer("   ");
            else
                printbuffer("       ");
            break;

        case TRACEC: /* tracecoords() */
            if (!depth)
                printbuffer("    ");
            break;

        case TRACEI: /* traceinherit() */
            if (!depth)
                printbuffer("\n    ");
            break;
    }
}

/* Dummy function to force Tcl/Tk to look at event queue to see of
   the user wants to stop the search. */

void WordNet::interface_doevents(void) {
    if (interface_doevents_func != NULL) interface_doevents_func();
}

/* Open exception list files */

int WordNet::morphinit(void) {
    //    int done = 0;
    //   int openerr = 0;
    /*
        if (!done) {
          if (!OpenDB) {		// make sure WN database files are open
                do_init();
                    done = 1;
              }
            } else
                openerr = -1;
        }
     */
    return 0;
    // return(openerr);
}

/* Close exception list files and reopen */
int WordNet::re_morphinit(void) {
    int i;

    for (i = 1; i <= NUMPARTS; i++) {
        if (exc_fps[i] != NULL) {
            fclose(exc_fps[i]);
            exc_fps[i] = NULL;
        }
    }

    if (!OpenDB) {
        do_init(NULL);
        return 0;
    }

    return -1;
}

/* Try to find baseform (lemma) of word or collocation in POS.
   Works like strtok() - first call is with string, subsequent calls
   with NULL argument return additional baseforms for original string. */

char *WordNet::morphstr(char *origstr, int pos) {
    static char searchstr[WORDBUF], str[WORDBUF];
    int svcnt = 0, svprep = 0;
    char word[WORDBUF], *tmp;
    int cnt, st_idx = 0, end_idx;
    int prep;
    char *end_idx1, *end_idx2;
    char *append;

    if (pos == SATELLITE)
        pos = ADJ;

    /* First time through for this string */

    if (origstr != NULL) {
        /* Assume string hasn't had spaces substitued with '_' */
        strcpy(str, WORDBUF, origstr);
        strtolower(strsubst(str, ' ', '_'));
        searchstr[0] = '\0';
        cnt = cntwords(str, '_');
        svprep = 0;

        /* first try exception list */

        if (((tmp = exc_lookup(str, pos)) != NULL) && strcmp(tmp, str)) {
            svcnt = 1; /* force next time to pass NULL */
            return (tmp);
        }

        /* Then try simply morph on original string */

        if (pos != VERB && ((tmp = morphword(str, pos)) != NULL) && strcmp(tmp, str))
            return (tmp);

        if (pos == VERB && cnt > 1 && ((prep = hasprep(str, cnt)) != NULL)) {
            /* assume we have a verb followed by a preposition */
            svprep = prep;
            return (morphprep(str));
        } else {
            svcnt = cnt = cntwords(str, '-');
            while (origstr && --cnt) {
                end_idx1 = strchr(str + st_idx, '_');
                end_idx2 = strchr(str + st_idx, '-');
                if (end_idx1 && end_idx2) {
                    if (end_idx1 < end_idx2) {
                        end_idx = (int) (end_idx1 - str);
                        append = "_";
                    } else {
                        end_idx = (int) (end_idx2 - str);
                        append = "-";
                    }
                } else {
                    if (end_idx1) {
                        end_idx = (int) (end_idx1 - str);
                        append = "_";
                    } else {
                        end_idx = (int) (end_idx2 - str);
                        append = "-";
                    }
                }
                if (end_idx < 0) return (NULL); /* shouldn't do this */
                strncpy_s(word, WORDBUF, str + st_idx, end_idx - st_idx);
                word[end_idx - st_idx] = '\0';
                if ((tmp = morphword(word, pos)) != NULL)
                    strcat(searchstr, WORDBUF, tmp);
                else
                    strcat(searchstr, WORDBUF, word);
                strcat(searchstr, WORDBUF, append);
                st_idx = end_idx + 1;
            }

            strcpy(word, WORDBUF, str + st_idx);
            if ((tmp = morphword(word, pos)) != NULL)
                strcat(searchstr, WORDBUF, tmp);
            else
                strcat(searchstr, WORDBUF, word);
            if (strcmp(searchstr, str) && is_defined(searchstr, pos))
                return (searchstr);
            else
                return (NULL);
        }
    } else { /* subsequent call on string */
        if (svprep) { /* if verb has preposition, no more morphs */
            svprep = 0;
            return (NULL);
        } else if (svcnt == 1)
            return (exc_lookup(NULL, pos));
        else {
            svcnt = 1;
            if (((tmp = exc_lookup(str, pos)) != NULL) && strcmp(tmp, str))
                return (tmp);
            else
                return (NULL);
        }
    }
}

/* Try to find baseform (lemma) of individual word in POS */
char *WordNet::morphword(char *word, int pos) {
    int offset, cnt;
    int i;
    static char retval[WORDBUF];
    char *tmp, tmpbuf[WORDBUF], *end;

    sprintf_s(retval, WORDBUF, "");
    sprintf_s(tmpbuf, WORDBUF, "");
    end = "";

    if (word == NULL)
        return (NULL);

    /* first look for word on exception list */

    if ((tmp = exc_lookup(word, pos)) != NULL)
        return (tmp); /* found it in exception list */

    if (pos == ADV) { /* only use exception list for adverbs */
        return (NULL);
    }
    if (pos == NOUN) {
        if (strend(word, "ful")) {
            cnt = strrchr(word, 'f') - word;
            strncat_s(tmpbuf, word, cnt);
            end = "ful";
        } else
            /* check for noun ending with 'ss' or short words */
            if (strend(word, "ss") || (strlen(word) <= 2))
            return (NULL);
    }

    /* If not in exception list, try applying rules from tables */

    if (tmpbuf[0] == '\0')
        strcpy(tmpbuf, WORDBUF, word);

    offset = offsets[pos];
    cnt = cnts[pos];

    for (i = 0; i < cnt; i++) {
        strcpy(retval, WORDBUF, wordbase(tmpbuf, (i + offset)));
        if (strcmp(retval, tmpbuf) && is_defined(retval, pos)) {
            strcat(retval, WORDBUF, end);
            return (retval);
        }
    }
    return (NULL);
}

int WordNet::strend(char *str1, char *str2) {
    char *pt1;

    if (strlen(str2) >= strlen(str1))
        return (0);
    else {
        pt1 = str1;
        pt1 = strchr(str1, 0);
        pt1 = pt1 - strlen(str2);
        return (!strcmp(pt1, str2));
    }
}

char *WordNet::wordbase(char *word, int ender) {
    char *pt1;
    static char copy[WORDBUF];

    strcpy(copy, WORDBUF, word);
    if (strend(copy, sufx[ender])) {
        pt1 = strchr(copy, '\0');
        pt1 -= strlen(sufx[ender]);
        *pt1 = '\0';
        strcat(copy, WORDBUF, addr[ender]);
    }
    return (copy);
}

int WordNet::hasprep(char *s, int wdcnt) {
    /* Find a preposition in the verb string and return its
       corresponding word number. */

    int i, wdnum;

    for (wdnum = 2; wdnum <= wdcnt; wdnum++) {
        s = strchr(s, '_');
        for (s++, i = 0; i < NUMPREPS; i++)
            if (!strncmp(s, prepositions[i].str, prepositions[i].strlen) &&
                    (s[prepositions[i].strlen] == '_' ||
                    s[prepositions[i].strlen] == '\0'))
                return (wdnum);
    }
    return (0);
}

char *WordNet::exc_lookup(char *word, int pos) {
    char line[WORDBUF], *beglp, *endlp;
    char *excline;
    //    int found = 0;

    if (exc_fps[pos] == NULL)
        return (NULL);

    /* first time through load line from exception file */
    if (word != NULL) {
        if ((excline = bin_search(word, exc_fps[pos])) != NULL) {
            strcpy(line, WORDBUF, excline);
            endlp = strchr(line, ' ');
        } else
            endlp = NULL;
    }
    if (endlp && *(endlp + 1) != ' ') {
        beglp = endlp + 1;
        while (*beglp && *beglp == ' ') beglp++;
        endlp = beglp;
        while (*endlp && *endlp != ' ' && *endlp != '\n') endlp++;
        if (endlp != beglp) {
            *endlp = '\0';
            return (beglp);
        }
    }
    beglp = NULL;
    endlp = NULL;
    return (NULL);
}

char *WordNet::morphprep(char *s) {
    char *rest, *exc_word, *lastwd = NULL, *last;
    int i, offset, cnt;
    char word[WORDBUF], end[WORDBUF];
    static char retval[WORDBUF];

    /* Assume that the verb is the first word in the phrase.  Strip it
       off, check for validity, then try various morphs with the
       rest of the phrase tacked on, trying to find a match. */

    rest = strchr(s, '_');
    last = strrchr(s, '_');
    if (rest != last) { /* more than 2 words */
        if ((lastwd = morphword(last + 1, NOUN)) != NULL) {
            strncpy_s(end, rest, last - rest + 1);
            end[last - rest + 1] = '\0';
            strcat(end, WORDBUF, lastwd);
        }
    }

    strncpy_s(word, s, rest - s);
    word[rest - s] = '\0';
    for (i = 0, cnt = strlen(word); i < cnt; i++)
        if (!isalnum((unsigned char) (word[i]))) return (NULL);

    offset = offsets[VERB];
    cnt = cnts[VERB];

    /* First try to find the verb in the exception list */

    if (((exc_word = exc_lookup(word, VERB)) != NULL) &&
            strcmp(exc_word, word)) {

        sprintf_s(retval, WORDBUF, "%s%s", exc_word, rest);
        if (is_defined(retval, VERB))
            return (retval);
        else if (lastwd) {
            sprintf_s(retval, WORDBUF, "%s%s", exc_word, end);
            if (is_defined(retval, VERB))
                return (retval);
        }
    }

    for (i = 0; i < cnt; i++) {
        if (((exc_word = wordbase(word, (i + offset))) != NULL) &&
                strcmp(word, exc_word)) { /* ending is different */

            sprintf_s(retval, WORDBUF, "%s%s", exc_word, rest);
            if (is_defined(retval, VERB))
                return (retval);
            else if (lastwd) {
                sprintf_s(retval, WORDBUF, "%s%s", exc_word, end);
                if (is_defined(retval, VERB))
                    return (retval);
            }
        }
    }
    sprintf_s(retval, WORDBUF, "%s%s", word, rest);
    if (strcmp(s, retval))
        return (retval);
    if (lastwd) {
        sprintf_s(retval, WORDBUF, "%s%s", word, end);
        if (strcmp(s, retval))
            return (retval);
    }
    return (NULL);
}


/* used by the strstr wrapper functions */
static char *strstr_word;
static char *strstr_stringstart;
static char *strstr_stringcurrent;

/* Initialization functions */


int WordNet::wninit(const char *wn_dir) {
    int done = 0;
    int openerr = 0;
    //    char env[256];
    //	size_t esize;

    if (!done) {
        /*if (getenv_s(&esize, env, 256, "WNDBVERSION") == 0)
        {
                wnrelease = _strdup(env);	// set release
                assert(wnrelease);
        }*/

        do_init(wn_dir);
        done = 1;
        OpenDB = 1;
        // openerr = morphinit();
    }

    return (openerr);
}

int WordNet::re_wninit(void) {
    int openerr;
    //    char *env;
    //	size_t envsize;

    closefps();

    /*_dupenv_s(&env, &envsize, "WNDBVERSION");
    if (env != NULL) {
    wnrelease = _strdup(env);	// set release 
    assert(wnrelease);
}*/

    do_init(NULL);
    OpenDB = 1;
    openerr = re_morphinit();

    return (openerr);
}

void WordNet::closefps(void) {
    int i;

    if (OpenDB) {
        for (i = 1; i < NUMPARTS + 1; i++) {
            if (datafps[i] != NULL)
                fclose(datafps[i]);
            datafps[i] = NULL;
            if (indexfps[i] != NULL)
                fclose(indexfps[i]);
            indexfps[i] = NULL;
        }
        if (sensefp != NULL) {
            fclose(sensefp);
            sensefp = NULL;
        }
        if (cntlistfp != NULL) {
            fclose(cntlistfp);
            cntlistfp = NULL;
        }
        if (keyindexfp != NULL) {
            fclose(keyindexfp);
            keyindexfp = NULL;
        }
        if (vsentfilefp != NULL) {
            fclose(vsentfilefp);
            vsentfilefp = NULL;
        }
        if (vidxfilefp != NULL) {
            fclose(vidxfilefp);
            vidxfilefp = NULL;
        }
        OpenDB = 0;

        //	int closed = _fcloseall();

    }
}

void WordNet::do_init(const char *wn_dir) {
    int i;
    char searchdir[256], tmpbuf[256];

    /* Find base directory for database.  If set, use WNSEARCHDIR.
       If not set, check for WNHOME/dict, otherwise use DEFAULTPATH. */

    if (wn_dir == NULL)
        sprintf_s(searchdir, 256, DEFAULTPATH);
    else
        sprintf_s(searchdir, 256, wn_dir);

    for (i = 1; i < NUMPARTS + 1; i++) {
        sprintf_s(tmpbuf, 256, DATAFILE, searchdir, partnames[i]);
        if ((fopen_s(&datafps[i], tmpbuf, "r")) != 0) {
            sprintf_s(msgbuf, 256,
                    "WordNet library error: Can't open datafile(%s)\n",
                    tmpbuf);
            throw WNException(msgbuf);
        }
        sprintf_s(tmpbuf, 256, INDEXFILE, searchdir, partnames[i]);
        if ((fopen_s(&indexfps[i], tmpbuf, "r")) != 0) {
            sprintf_s(msgbuf, 256,
                    "WordNet library error: Can't open indexfile(%s)\n",
                    tmpbuf);
            throw WNException(msgbuf);
        }
    }

    /* This file isn't used by the library and doesn't have to
       be present.  No error is reported if the open fails. */

    sprintf_s(tmpbuf, 256, SENSEIDXFILE, searchdir);
    fopen_s(&sensefp, tmpbuf, "r");

    /* If this file isn't present, the runtime code will skip printint out
       the number of times each sense was tagged. */

    sprintf_s(tmpbuf, 256, CNTLISTFILE, searchdir);
    fopen_s(&cntlistfp, tmpbuf, "r");

    /* This file doesn't have to be present.  No error is reported if the
       open fails. */

    sprintf_s(tmpbuf, 256, KEYIDXFILE, searchdir);
    fopen_s(&keyindexfp, tmpbuf, "r");

    sprintf_s(tmpbuf, 256, REVKEYIDXFILE, searchdir);
    fopen_s(&revkeyindexfp, tmpbuf, "r");

    sprintf_s(tmpbuf, 256, VRBSENTFILE, searchdir);
    if ((fopen_s(&vsentfilefp, tmpbuf, "r")) != 0) {
        sprintf_s(msgbuf, 256,
                "WordNet library warning: Can't open verb example sentence file(%s)\n",
                tmpbuf);
        throw WNException(msgbuf);
    }

    sprintf_s(tmpbuf, 256, VRBIDXFILE, searchdir);
    if ((fopen_s(&vidxfilefp, tmpbuf, "r")) != 0) {
        sprintf_s(msgbuf, 256,
                "WordNet library warning: Can't open verb example sentence index file(%s)\n",
                tmpbuf);
        throw WNException(msgbuf);
    }

    return;
}

/* Count the number of underscore or space separated words in a string. */

int WordNet::cntwords(char *s, char separator) {
    register int wdcnt = 0;

    while (*s) {
        if (*s == separator || *s == ' ' || *s == '_') {
            wdcnt++;
            while (*s && (*s == separator || *s == ' ' || *s == '_'))
                s++;
        } else
            s++;
    }
    return (++wdcnt);
}

/* Convert string to lower case remove trailing adjective marker if found */

char *WordNet::strtolower(char *str) {
    register char *s = str;

    while (*s != '\0') {
        if (*s >= 'A' && *s <= 'Z')
            *s += 32;
        else if (*s == '(') {
            *s = '\0';
            break;
        }
        s++;
    }
    return (str);
}

/* Convert string passed to lower case */

char *WordNet::ToLowerCase(char *str) {
    register char *s = str;

    while (*s != '\0') {
        if (*s >= 'A' && *s <= 'Z')
            *s += 32;
        s++;
    }
    return (str);
}

/* Replace all occurences of 'from' with 'to' in 'str' */

char *WordNet::strsubst(char *str, char from, char to) {
    register char *p;

    for (p = str; *p != 0; ++p)
        if (*p == from)
            *p = to;
    return str;
}

/* Return pointer code for pointer type characer passed. */

int WordNet::getptrtype(char *ptrstr) {
    register int i;
    for (i = 1; i <= MAXPTR; i++) {
        if (!strcmp(ptrstr, ptrtyp[i]))
            return (i);
    }
    return (0);
}

/* Return part of speech code for string passed */

int WordNet::getpos(char *s) {
    switch (*s) {
        case 'n':
            return (NOUN);
        case 'a':
        case 's':
            return (ADJ);
        case 'v':
            return (VERB);
        case 'r':
            return (ADV);
        default:
            sprintf_s(msgbuf, 256,
                    "WordNet library error: unknown part of speech %s\n", s);
            display_message(msgbuf);
            exit(-1);
    }
}

/* Return synset type code for string passed. */

int WordNet::getsstype(char *s) {
    switch (*s) {
        case 'n':
            return (NOUN);
        case 'a':
            return (ADJ);
        case 'v':
            return (VERB);
        case 's':
            return (SATELLITE);
        case 'r':
            return (ADV);
        default:
            sprintf_s(msgbuf, 256, "WordNet library error: Unknown synset type %s\n", s);
            display_message(msgbuf);
            exit(-1);
    }
}

/* Pass in string for POS, return corresponding integer value */

int WordNet::StrToPos(char *str) {
    if (!strcmp(str, "noun"))
        return (NOUN);
    else if (!strcmp(str, "verb"))
        return (VERB);
    else if (!strcmp(str, "adj"))
        return (ADJ);
    else if (!strcmp(str, "adv"))
        return (ADV);
    else {
        return (-1);
    }
}

#define MAX_TRIES	5

/* Find string for 'searchstr' as it is in index file */

char *WordNet::GetWNStr(char *searchstr, int dbase) {
    register int i, j, k, offset = 0;
    register char c;
    char *underscore = NULL, *hyphen = NULL, *period = NULL;
    char strings[MAX_TRIES][WORDBUF];

    ToLowerCase(searchstr);

    if (((underscore = strchr(searchstr, '_')) == NULL) &&
            ((hyphen = strchr(searchstr, '-')) == NULL) &&
            ((period = strchr(searchstr, '.'))) == NULL)
        strcpy(strings[0], WORDBUF, searchstr);
    return strings[0];

    for (i = 0; i < 3; i++)
        strcpy(strings[i], WORDBUF, searchstr);
    if (underscore != NULL) strsubst(strings[1], '_', '-');
    if (hyphen != NULL) strsubst(strings[2], '-', '_');
    for (i = j = k = 0; (c = searchstr[i]) != '\0'; i++) {
        if (c != '_' && c != '-') strings[3][j++] = c;
        if (c != '.') strings[4][k++] = c;
    }
    strings[3][j] = '\0';
    strings[4][k] = '\0';

    for (i = 1; i < MAX_TRIES; i++)
        if (strcmp(strings[0], strings[i]) == 0) strings[i][0] = '\0';

    for (i = (MAX_TRIES - 1); i >= 0; i--)
        if (strings[i][0] != '\0')
            if (bin_search(strings[i], indexfps[dbase]) != NULL)
                offset = i;

    return (strings[offset]);
}

/* Return synset for sense key passed. */

SynsetPtr WordNet::GetSynsetForSense(char *sensekey) {
    long offset;

    /* Pass in sense key and return parsed sysnet structure */

    if ((offset = GetDataOffset(sensekey)) != NULL)
        return (read_synset(GetPOS(sensekey),
            offset,
            GetWORD(sensekey)));
    else
        return (NULL);
}

/* Find offset of sense key in data file */

long WordNet::GetDataOffset(char *sensekey) {
    char *line;

    /* Pass in encoded sense string, return byte offset of corresponding
       synset in data file. */

    if (sensefp == NULL) {
        display_message("WordNet library error: Sense index file not open\n");
        return (0L);
    }
    line = bin_search(sensekey, sensefp);
    if (line) {
        while (*line++ != ' ');
        return (atol(line));
    } else
        return (0L);
}

/* Find polysemy count for sense key passed. */

int WordNet::GetPolyCount(char *sensekey) {
    IndexPtr idx;
    int sense_cnt = 0;

    /* Pass in encoded sense string and return polysemy count
       for word in corresponding POS */

    idx = index_lookup(GetWORD(sensekey), GetPOS(sensekey));
    if (idx) {
        sense_cnt = idx->sense_cnt;
        free_index(idx);
    }
    return (sense_cnt);
}

/* Return word part of sense key */
char *WordNet::GetWORD(char *sensekey) {
    static char word[100];
    int i = 0;

    /* Pass in encoded sense string and return WORD */

    while ((word[i++] = *sensekey++) != '%');
    word[i - 1] = '\0';
    return (word);
}

/* Return POS code for sense key passed. */

int WordNet::GetPOS(char *sensekey) {
    int pos;

    /* Pass in encoded sense string and return POS */

    while (*sensekey++ != '%'); /* skip over WORD */
    sscanf_s(sensekey, "%1d", &pos);
    return (pos == SATELLITE ? ADJ : pos);
}

/* Reconstruct synset from synset pointer and return ptr to buffer */

char *WordNet::FmtSynset(SynsetPtr synptr, int defn) {
    int i;
    static char synset[SMLINEBUF];

    synset[0] = '\0';

    if (fileinfoflag)
        sprintf_s(synset, SMLINEBUF, "<%s> ", lexfiles[synptr->fnum]);

    strcat(synset, SMLINEBUF, "{ ");
    for (i = 0; i < (synptr->wcount - 1); i++)
        sprintf_s(synset + strlen(synset), SMLINEBUF - strlen(synset), "%s, ", synptr->words[i]);

    strcat(synset, SMLINEBUF, synptr->words[i]);

    if (defn && synptr->defn)
        sprintf_s(synset + strlen(synset), SMLINEBUF - strlen(synset), " (%s) ", synptr->defn);

    strcat(synset, SMLINEBUF, " }");
    return (synset);
}

/* Convert WordNet sense number passed of IndexPtr entry to sense key. */
char *WordNet::WNSnsToStr(IndexPtr idx, int sense) {
    SynsetPtr sptr, adjss;
    char sensekey[512], lowerword[256];
    int j, sstype, pos;

    pos = getpos(idx->pos);
    sptr = read_synset(pos, idx->offset[sense - 1], "");

    if ((sstype = getsstype(sptr->pos)) == SATELLITE) {
        for (j = 0; j < sptr->ptrcount; j++) {
            if (sptr->ptrtyp[j] == SIMPTR) {
                adjss = read_synset(sptr->ppos[j], sptr->ptroff[j], "");
                sptr->headword = (char *) malloc(strlen(adjss->words[0]) + 1);
                assert(sptr->headword);
                strcpy(sptr->headword, strlen(adjss->words[0]) + 1, adjss->words[0]);
                strtolower(sptr->headword);
                sptr->headsense = (short) adjss->lexid[0];
                free_synset(adjss);
                break;
            }
        }
    }

    for (j = 0; j < sptr->wcount; j++) {
        strcpy(lowerword, 256, sptr->words[j]);
        strtolower(lowerword);
        if (!strcmp(lowerword, idx->wd))
            break;
    }

    if (j == sptr->wcount) {
        free_synset(sptr);
        return (NULL);
    }

    if (sstype == SATELLITE)
        sprintf_s(sensekey, 512, "%s%%%-1.1d:%-2.2d:%-2.2d:%s:%-2.2d",
            idx->wd, SATELLITE, sptr->fnum,
            sptr->lexid[j], sptr->headword, sptr->headsense);
    else
        sprintf_s(sensekey, 512, "%s%%%-1.1d:%-2.2d:%-2.2d::",
            idx->wd, pos, sptr->fnum, sptr->lexid[j]);

    free_synset(sptr);
    return (_strdup(sensekey));
}

/* Search for string and/or baseform of word in database and return
   index structure for word if found in database. */

IndexPtr WordNet::GetValidIndexPointer(char *word, int pos) {
    IndexPtr idx;
    char *morphword;

    idx = getindex(word, pos);

    if (idx == NULL) {
        if ((morphword = morphstr(word, pos)) != NULL)
            while (morphword) {
                if ((idx = getindex(morphword, pos)) != NULL) break;
                morphword = morphstr(NULL, pos);
            }
    }
    return (idx);
}

/* Return sense number in database for word and lexsn passed. */

int WordNet::GetWNSense(char *word, char *lexsn) {
    SnsIndexPtr snsidx;
    char buf[256];

    sprintf_s(buf, 256, "%s%%%s", word, lexsn); /* create sensekey */
    if ((snsidx = GetSenseIndex(buf)) != NULL)
        return (snsidx->wnsense);
    else
        return (0);
}

/* Return parsed sense index entry for sense key passed. */

SnsIndexPtr WordNet::GetSenseIndex(char *sensekey) {
    char *line;
    char buf[256], loc[9];
    SnsIndexPtr snsidx = NULL;

    if ((line = bin_search(sensekey, sensefp)) != NULL) {
        snsidx = (SnsIndexPtr) malloc(sizeof (SnsIndex));
        assert(snsidx);
        sscanf_s(line, "%s %s %d %d\n",
                buf,
                loc,
                &snsidx->wnsense,
                &snsidx->tag_cnt);
        snsidx->sensekey = (char *) malloc(strlen(buf) + 1);
        assert(snsidx->sensekey);
        strcpy(snsidx->sensekey, strlen(buf) + 1, buf);
        snsidx->loc = atol(loc);
        /* Parse out word from sensekey to make things easier for caller */
        snsidx->word = _strdup(GetWORD(snsidx->sensekey));
        assert(snsidx->word);
        snsidx->nextsi = NULL;
    }
    return (snsidx);
}

/* Return number of times sense is tagged */

int WordNet::GetTagcnt(IndexPtr idx, int sense) {
    char *sensekey, *line;
    char buf[256];
    int snum, cnt = 0;

    if (cntlistfp) {

        sensekey = WNSnsToStr(idx, sense);
        if ((line = bin_search(sensekey, cntlistfp)) != NULL) {
            sscanf_s(line, "%s %d %d", buf, &snum, &cnt);
        }
        free(sensekey);
    }

    return (cnt);
}

void WordNet::FreeSenseIndex(SnsIndexPtr snsidx) {
    if (snsidx) {
        free(snsidx->word);
        free(snsidx);
    }
}

char *WordNet::GetOffsetForKey(unsigned int key) {
    unsigned int rkey;
    char ckey[7];
    static char loc[11] = "";
    char *line;
    char searchdir[256], tmpbuf[256];

    /* Try to open file in case wn_init wasn't called */

    if (!keyindexfp) {
        strcpy(searchdir, 256, SetSearchdir());
        sprintf_s(tmpbuf, 256, KEYIDXFILE, searchdir);
        fopen_s(&keyindexfp, tmpbuf, "r");
    }
    if (keyindexfp) {
        sprintf_s(ckey, 7, "%6.6d", key);
        if ((line = bin_search(ckey, keyindexfp)) != NULL) {
            sscanf_s(line, "%d %s", &rkey, loc);
            return (loc);
        }
    }
    return (NULL);
}

unsigned int WordNet::GetKeyForOffset(char *loc) {
    unsigned int key;
    char rloc[11] = "";
    char *line;
    char searchdir[256], tmpbuf[256];

    /* Try to open file in case wn_init wasn't called */

    if (!revkeyindexfp) {
        strcpy(searchdir, 256, SetSearchdir());
        sprintf_s(tmpbuf, 256, REVKEYIDXFILE, searchdir);
        fopen_s(&revkeyindexfp, tmpbuf, "r");
    }
    if (revkeyindexfp) {
        if ((line = bin_search(loc, revkeyindexfp)) != NULL) {
            sscanf_s(line, "%s %d", rloc, &key);
            return (key);
        }
    }
    return (0);
}

char *WordNet::SetSearchdir() {
    static char searchdir[256];
    char *env;
    size_t envsize;

    /* Find base directory for database.  If set, use WNSEARCHDIR.
       If not set, check for WNHOME/dict, otherwise use DEFAULTPATH. */

    _dupenv_s(&env, &envsize, "WNSEARCHDIR");
    if (env != NULL)
        strcpy(searchdir, 256, env);
    else {
        _dupenv_s(&env, &envsize, "WNHOME");
        if (env != NULL)
            sprintf_s(searchdir, 256, "%s%s", env, DICTDIR);
        else
            strcpy(searchdir, 256, DEFAULTPATH);
    }

    return (searchdir);
}

int default_display_message(char *msg) {
    return (-1);
}

/* 
 ** Wrapper functions for strstr that allow you to retrieve each
 ** occurance of a word within a longer string, not just the first.
 **
 ** strstr_init is called with the same arguments as normal strstr,
 ** but does not return any value.
 **
 ** strstr_getnext returns the position offset (not a pointer, as does
 ** normal strstr) of the next occurance, or -1 if none remain.
 */

void WordNet::strstr_init(char *string, char *word) {
    strstr_word = word;
    strstr_stringstart = string;
    strstr_stringcurrent = string;
}

int WordNet::strstr_getnext(void) {
    char *loc = strstr(strstr_stringcurrent, strstr_word);
    if (loc == NULL) return -1;
    strstr_stringcurrent = loc + 1;
    return (loc - strstr_stringstart);
}


#define KEY_LEN		(1024)
#define LINE_LEN	(1024*25)

// static char line[LINE_LEN]; 
// long last_bin_search_offset = 0;

/* General purpose binary search function to search for key as first
   item on line in open file.  Item is delimited by space. */

#undef getc

char *WordNet::read_index(long offset, FILE *fp) {
    char *linep;

    linep = linebuf;
    linebuf[0] = '0';

    fseek(fp, offset, SEEK_SET);
    fgets(linep, LINEBUF, fp);
    return (linebuf);
}

char *WordNet::bin_search(char *searchkey, FILE *fp) {
    int c;
    long top, mid, bot, diff;
    char *linep, key[KEY_LEN];
    int length;

    diff = 666;
    linep = linebuf;
    linebuf[0] = '\0';

    fseek(fp, 0L, 2);
    top = 0;
    bot = ftell(fp);
    mid = (bot - top) / 2;

    do {
        fseek(fp, mid - 1, 0);
        if (mid != 1)
            while ((c = getc(fp)) != '\n' && c != EOF);
        last_bin_search_offset = ftell(fp);
        fgets(linep, LINEBUF, fp);
        length = (int) (strchr(linep, ' ') - linep);
        strncpy_s(key, KEY_LEN, linep, length);
        key[length] = '\0';
        if (strcmp(key, searchkey) < 0) {
            top = mid;
            diff = (bot - top) / 2;
            mid = top + diff;
        }
        if (strcmp(key, searchkey) > 0) {
            bot = mid;
            diff = (bot - top) / 2;
            mid = top + diff;
        }
    } while ((strcmp(key, searchkey)) && (diff != 0));

    if (!strcmp(key, searchkey))
        return (linebuf);
    else
        return (NULL);
}

static long offset;

int WordNet::bin_search_key(char *searchkey, char *line, int linelen, FILE *fp) {
    int c;
    long top, mid, bot, diff;
    char *linep, key[KEY_LEN];
    int length, offset1, offset2;

    /* do binary search to find correct place in file to insert line */

    diff = 666;
    linep = line;
    line[0] = '\0';

    fseek(fp, 0L, 2);
    top = 0;
    bot = ftell(fp);
    if (bot == 0) {
        offset = 0;
        return (0); /* empty file */
    }
    mid = (bot - top) / 2;

    /* If only one line in file, don't work through loop */

    length = 0;
    rewind(fp);
    while ((c = getc(fp)) != '\n' && c != EOF)
        line[length++] = (char) c;
    if (getc(fp) == EOF) { /* only 1 line in file */
        length = (int) (strchr(linep, ' ') - linep);
        strncpy_s(key, KEY_LEN, linep, length);
        key[length] = '\0';
        if (strcmp(key, searchkey) > 0) {
            offset = 0;
            return (0); /* line with key is not found */
        } else if (strcmp(key, searchkey) < 0) {
            offset = ftell(fp);
            return (0); /* line with key is not found */
        } else {
            offset = 0;
            return (1); /* line with key is found */
        }
    }

    do {
        fseek(fp, mid - 1, 0);
        if (mid != 1)
            while ((c = getc(fp)) != '\n' && c != EOF);
        offset1 = ftell(fp); /* offset at start of line */
        if (fgets(linep, linelen, fp) != NULL) {
            offset2 = ftell(fp); /* offset at start of next line */
            length = (int) (strchr(linep, ' ') - linep);
            strncpy_s(key, KEY_LEN, linep, length);
            key[length] = '\0';
            if (strcmp(key, searchkey) < 0) { /* further in file */
                top = mid;
                diff = (bot - top) / 2;
                mid = top + diff;
                offset = offset2;
            }
            if (strcmp(key, searchkey) > 0) { /* earlier in file */
                bot = mid;
                diff = (bot - top) / 2;
                mid = top + diff;
                offset = offset1;
            }
        } else {
            bot = mid;
            diff = (bot - top) / 2;
            mid = top + diff;
        }
    } while ((strcmp(key, searchkey)) && (diff != 0));

    if (!strcmp(key, searchkey)) {
        offset = offset1; /* get to start of current line */
        return (1); /* line with key is found */
    } else
        return (0); /* line with key is not found */
}

/* Copy contents from one file to another. */

void WordNet::copyfile(FILE *fromfp, FILE *tofp) {
    int c;

    while ((c = getc(fromfp)) != EOF)
        putc(c, tofp);
}

/* Function to replace a line in a file.  Returns the original line,
   or NULL in case of error. */

char *WordNet::replace_line(char *new_line, char *searchkey, FILE *fp) {
    FILE *tfp; /* temporary file pointer */

    if (!bin_search_key(searchkey, linebuf, LINEBUF, fp))
        return (NULL); /* line with key not found */

    if ((tmpfile_s(&tfp)) != 0)
        return (NULL); /* could not create temp file */
    fseek(fp, offset, 0);
    fgets(linebuf, LINE_LEN, fp); /* read original */
    copyfile(fp, tfp);
    if (fseek(fp, offset, 0) == -1)
        return (NULL); /* could not seek to offset */
    fprintf(fp, new_line); /* write line */
    rewind(tfp);
    copyfile(tfp, fp);

    fclose(tfp);
    fflush(fp);

    return (linebuf);
}

/* Find location to insert line at in file.  If line with this
   key is already in file, return NULL. */

char *WordNet::insert_line(char *new_line, char *searchkey, FILE *fp) {
    FILE *tfp;
    char linebuf[LINE_LEN];

    if (bin_search_key(searchkey, linebuf, LINE_LEN, fp))
        return (NULL);

    if ((tmpfile_s(&tfp)) == 0)
        return (NULL); /* could not create temp file */
    if (fseek(fp, offset, 0) == -1)
        return (NULL); /* could not seek to offset */
    copyfile(fp, tfp);
    if (fseek(fp, offset, 0) == -1)
        return (NULL); /* could not seek to offset */
    fprintf(fp, new_line); /* write line */
    rewind(tfp);
    copyfile(tfp, fp);

    fclose(tfp);
    fflush(fp);

    return (new_line);
}

float WordNet::GetSpecificity(long offset) {
    SynsetPtr cursynset = read_synset(NOUN, offset, NULL);
    if (cursynset == NULL)
        return 0;

    if (cursynset->ptrcount == 0)
        return 0;

    float result = 0.0f;
    int count = 0;
    long new_offset;
    for (int i = 0; i < cursynset->ptrcount; i++) {
        if (cursynset->ptrtyp[i] != HYPERPTR)
            continue;

        new_offset = cursynset->ptroff[i];
        result += GetSpecificity(new_offset);
        count++;
    }

    if (count == 0)
        return 1;
    return (result / count) + 1;
}

#endif // def USE_WORDNET
