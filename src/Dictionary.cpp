/* 
 * File:   Dictionary.cpp
 * Author: root
 * 
 * Created on October 4, 2012, 3:10 PM
 */

#ifdef _WIN32
#include "stdafx.h"
#endif

#ifdef USE_WORDNET

#include "Dictionary.h"
#include "tinyxml.h"
#include "wn.h"

namespace util {
    
SynsetInfo::SynsetInfo() : parent(0), root(0), has_images(false) {
};

SynsetInfo::SynsetInfo(const SynsetInfo &copy_from) {
    // offset = copy_from.offset;
    words = copy_from.words;
    gloss = copy_from.gloss;
    parent = copy_from.parent;
    root = copy_from.root;
    has_images = copy_from.has_images;
}

SynsetInfo::~SynsetInfo() {
};

/*bool SynsetInfo::operator < (const SynsetInfo &right) const
{
        return (offset < right.offset);
}

bool SynsetInfo::operator == (const SynsetInfo &right) const
{
        return (offset == right.offset);
}

bool SynsetInfo::operator != (const SynsetInfo &right) const
{
        return !(offset == right.offset);
}*/

Dictionary::Dictionary(const char *wn_dir, const char *imgnet_file, const char *structfile) {
    init(wn_dir, imgnet_file, structfile);
}

Dictionary::~Dictionary(void) {
    
}

bool Dictionary::hasNoun(const char *word) {
    if ((word == NULL) || (strlen(word) == 0))
        throw util::RuntimeException("(Dictionary::hasNoun) Null or empty word passed in.");
    
    char *buf = new char[strlen(word)+1];
    strncpy(buf, word, strlen(word));
    bool result = (is_defined(buf, NOUN) > 0);
    delete[] buf;
    return result;
}

bool Dictionary::getSenseOffsets(const char *word, std::vector<long> &offsetlist) {
    char wordbuf[256];
    wordbuf[255] = '\0';
    strncpy(wordbuf, word, 255);

    offsetlist.clear();

    SynsetPtr ssptr = NULL;

    if ((ssptr = findtheinfo_ds(wordbuf, NOUN, CLASSIFICATION, ALLSENSES)) == NULL)
        return false;

    SynsetPtr nextsns;
    for (nextsns = ssptr; nextsns != NULL; nextsns = nextsns->nextss) {
        offsetlist.push_back(nextsns->hereiam);
    }
    delete ssptr;

    return true;
}

void Dictionary::init(const char *wn_dir, const char *imgnet_file, const char *structfile) {
    std::cout << DEFAULTPATH << "\n";
    
    wninit();
    loadStructFile(structfile);
    loadSynsetList(imgnet_file);
}

void Dictionary::loadSynsetList(const char *imgnet_file) {
    std::ifstream ssfile;
    ssfile.open(imgnet_file);
    if (!ssfile.is_open())
        throw util::IOException("(Dictionary:LoadSynsetList) Unable to open synset list file for reading");

    std::string line;
    long offset;
    std::map<long, SynsetInfo>::iterator results;
    SynsetInfo findss;
    char buf[1024];
    while (!ssfile.eof()) {
        buf[1023] = '\0';
        ssfile.getline(buf, 1023);
        line = buf;
        if (line.size() == 0)
            continue;
        line = line.substr(1, line.size());
        offset = strtol(line.c_str(), NULL, 10);

        // Insert missing synsets under "Misc" (0)
        if ((results = _img_synsets.find(offset)) == _img_synsets.end()) {
            std::pair<long, SynsetInfo> ins_val;
            ins_val.first = offset;
            ins_val.second.parent = 0;
            ins_val.second.root = 0;
            _img_synsets.insert(ins_val);
        } else
            results->second.has_images = true;
    } 

    ssfile.close();
}

bool Dictionary::synsetHasImages(long offset) {
    std::map<long, SynsetInfo>::iterator it = _img_synsets.find(offset);
    if (it == _img_synsets.end())
        return false;
    return it->second.has_images;
}

long Dictionary::getRootOffset(long offset) {
    std::map<long, SynsetInfo>::iterator s_it = _img_synsets.find(offset);

    if (s_it == _img_synsets.end())
        return 0;
    return s_it->second.root;
}

void Dictionary::loadStructFile(const char *structfile) {
    _img_synsets.clear();

    std::string filename = structfile;
    TiXmlDocument xmldoc(filename.c_str());

    if (!xmldoc.LoadFile())
        throw util::IOException("(Dictionary:LoadStructFile) Could not load the ImageNet Struct XML file.");

    // Get the root element, should be a simple tag of ImageNetStructure
    TiXmlNode* sSynset = xmldoc.FirstChild();
    std::string buf = sSynset->Value();
    if (buf.compare("ImageNetStructure") != 0)
        throw util::IOException("(Dictionary:LoadStructFile) ImgNet Structure file incorrect format.");

    for (sSynset = sSynset->FirstChild(); sSynset != NULL; sSynset = sSynset->NextSibling()) {
        buf = sSynset->Value();
        if (buf.compare("releaseData") == 0)
            continue;
        else if (buf.compare("synset") == 0) {
            // Verify this is a valid root node
            TiXmlElement *eSynset;
            if ((eSynset = sSynset->ToElement()) == NULL)
                throw util::IOException("(Dictionary::LoadStructFile) ImgNet Structure had unexpected type");

            TiXmlAttribute *att = eSynset->FirstAttribute();
            int has_root = false;
            for (; att != NULL; att = att->Next()) {
                std::string attval = att->Value();
                if ((attval.compare("wnid") == 0) || (attval.compare("gproot") == 0))
                    has_root = true;
            }
            if (!has_root)
                throw util::IOException("(Dictionary::LoadStructFile) ImgNet Structure had unexpected type.");

            // Now get all the category roots and read them in
            TiXmlNode *catroots = sSynset->FirstChild();
            if (catroots == NULL)
                throw util::IOException("(Dictionary::LoadStructFile) ImgNet Structure had unexpected format.");

            for (; catroots != NULL; catroots = catroots->NextSibling()) {
                std::pair<long, SynsetInfo> new_synset;
                new_synset.first = loadSynsetInfo(new_synset.second, catroots, 0, 0);
                if ((new_synset.first == 3967323l) || (new_synset.first == 10573931l) || 
                    ((new_synset.first == 9609573l) || (new_synset.first == 9328648l)))
                    std::cout << "Found " << new_synset.first << "\n";
                
                _img_synsets.insert(new_synset);
                loadSynsetChildren(new_synset.first, catroots, new_synset.first);
            }
        }
    }

}

long Dictionary::loadSynsetInfo(SynsetInfo &dst, TiXmlNode *node, long parent, long root) {
    long offset;

    TiXmlElement *element = node->ToElement();
    if (element == NULL)
        throw util::RuntimeException("(Dictionary::LoadSynsetInfo) Xml node passed that was not an element");

    dst.parent = parent;
    dst.root = root;
    std::string offsetstr(element->Attribute("wnid"));
    if (offsetstr.size() <= 1)
        throw util::IOException("(Dictionary::LoadSynsetInfo) Xml element was missing proper wnid data");

    offset = atol(offsetstr.c_str() + 1);

    dst.gloss = element->Attribute("gloss");
    dst.words = element->Attribute("words");
    return offset;
}

void Dictionary::loadSynsetChildren(long p_offset, TiXmlNode *node, long root) {
    TiXmlElement *element = node->ToElement();
    if (element == NULL)
        throw util::RuntimeException("(Dictionary::LoadSynsetInfo) Xml node passed that was not an element");

    TiXmlNode *child;
    for (child = node->FirstChild(); child != NULL; child = child->NextSibling()) {
        std::pair<long, SynsetInfo> new_synset;
        new_synset.first = loadSynsetInfo(new_synset.second, child, p_offset, root);
        _img_synsets.insert(new_synset);
        // throw CodeException("(Dictionary:LoadSynsetChildren) Attempt to insert duplicate synsets");

        loadSynsetChildren(new_synset.first, child, root);
    }
}

bool Dictionary::getDefinition(const long offset, std::string &dst, const char *noun) {
    SynsetPtr ssptr = NULL;
    char nounstr[1024];
    if (noun != NULL)
        strncpy(nounstr, noun, 1024);
    else
        nounstr[0] = '\0';

    if ((ssptr = read_synset(NOUN, offset, nounstr)) == NULL)
        return false;

    dst = ssptr->defn;

    return true;

}

bool Dictionary::getWords(const long offset, std::string &dst, const char *noun) {
    SynsetPtr ssptr = NULL;
    char nounstr[1024];
    if (noun != NULL)
        strncpy(nounstr, noun, 1024);
    else
        nounstr[0] = '\0';

    if ((ssptr = read_synset(NOUN, offset, nounstr)) == NULL)
        return false;

    dst.clear();
    bool first = true;
    for (int i = 0; i < ssptr->wcount; i++) {
        if (!first)
            dst += ", ";
        dst += ssptr->words[i];
        first = false;
    }

    return true;
}

bool Dictionary::getWords(const long offset, std::vector<std::string> &dst, const char *noun, bool only_first) {
    SynsetPtr ssptr = NULL;
    char nounstr[1024];
    if (noun != NULL)
        strncpy(nounstr, noun, 1024);
    else
        nounstr[0] = '\0';

    if ((ssptr = read_synset(NOUN, offset, nounstr)) == NULL)
        return false;

    dst.clear();
    for (int i = 0; i < (only_first ? 1 : ssptr->wcount); i++) {
        dst.push_back(ssptr->words[i]);
    }

    return true;
}

float Dictionary::getSpecificity(const long offset) {
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
        result += getSpecificity(new_offset);
        count++;
    }

    if (count == 0)
        return 1;
    return (result / count) + 1;

    //	return wn.GetSpecificity(offset);
}

bool Dictionary::getPointers(const long offset, std::vector<std::pair<synset_type, long > > &dst, 
                                                        const char *noun) {
    char nounstr[1024];
    if (noun != NULL)
        strncpy(nounstr, noun, 1024);
    else
        nounstr[0] = '\0';

    SynsetPtr cursynset = read_synset(NOUN, offset, nounstr);
    if (cursynset == NULL)
        return false;

    if (cursynset->ptrcount == 0)
        return false;

    dst.clear();

    for (int i = 0; i < cursynset->ptrcount; i++) {
        std::pair<synset_type, long> newval;
        newval.second = cursynset->ptroff[i];
        
        switch (cursynset->ptrtyp[i]) {
            case HYPERPTR:
                newval.first = Hypernym;
                break;
            case HYPOPTR:
                newval.first = Hyponym;
                break;
            case ANTPTR:
                newval.first = Ant;
                break;
            case ENTAILPTR:
                newval.first = Entail;
                break;
            case SIMPTR:
                newval.first = Sim;
                break;
            default:
                newval.second = Unsupported;
                break;
        }
                

        dst.push_back(newval);
    }

    return true;
}

} // namespace util

#endif // USE_WORDNET
