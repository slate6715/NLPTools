/* 
 * File:   Dictionary.h
 * Author: root
 *
 * Created on October 4, 2012, 3:10 PM
 */

#ifndef DICTIONARY_H
#define	DICTIONARY_H
#include <vector>
#include <set>
#include <string>
#include <map>

#ifdef USE_WORDNET

class TiXmlNode;

#define PEOPLE_OFFSET	7846
namespace util {
    
class SynsetInfo {
public:
    SynsetInfo();
    //	SynsetInfo(long _offset);
    SynsetInfo(const SynsetInfo &copy_from);
    ~SynsetInfo();

    //	bool operator < (const SynsetInfo &right) const;
    //	bool operator == (const SynsetInfo &right) const;
    //	bool operator != (const SynsetInfo &right) const;

    // long offset;
    std::string words;
    std::string gloss;
    long parent;
    long root;
    bool has_images;
};

class Dictionary {
public:
    Dictionary(const char *wn_dir, const char *imgnet_file, const char *structfile);
    virtual ~Dictionary(void);

    enum synset_type { Hypernym, Hyponym, Ant, Entail, Sim, Unsupported };
  
    void init(const char *wn_dir, const char *imgnet_file, const char *structfile);

    bool hasNoun(const char *word);
    bool getSenseOffsets(const char *word, std::vector<long> &offsetlist);
    long getRootOffset(long offset);

    bool synsetHasImages(long offset);
    bool getDefinition(const long offset, std::string &dst, const char *noun);
    bool getWords(const long offset, std::string &dst, const char *noun);
    bool getWords(const long offset, std::vector<std::string> &dst, const char *noun, bool only_first = false);
    float getSpecificity(const long offset);
    bool getPointers(const long offset, std::vector<std::pair<synset_type, long > > &dst, const char *noun);

private:
    void loadSynsetList(const char *imgnet_file);
    void loadStructFile(const char *structfile);
    long loadSynsetInfo(SynsetInfo &dst, TiXmlNode *node, long parent, long root);
    void loadSynsetChildren(long p_offset, TiXmlNode *node, long root);

    bool _initialized;

    // Image synsets available, with root synset indicated
    std::map<long, SynsetInfo> _img_synsets;
};
 
}

#endif // USE_WORDNET

#endif	/* DICTIONARY_H */

