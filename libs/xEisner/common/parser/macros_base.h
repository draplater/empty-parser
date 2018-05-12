#ifndef _MACROS_H
#define _MACROS_H

#include <set>
#include <tuple>
#include <vector>

#include "include/ngram.h"
#include "common/token/token.h"
#include "include/learning/perceptron/packed_score.h"

#define ROOT_WORD		"-ROOT-"
#define ROOT_POSTAG		"-ROOT-"
#define ROOT_DEPLABEL	"-ROOT-"
#define EMPTY_WORD		"-EMPTY-"
#define EMPTY_POSTAG	"-EMPTY-"
#define START_WORD		"-START-"
#define START_POSTAG	"-START-"
#define MIDDLE_WORD		"-MIDDLE-"
#define MIDDLE_POSTAG	"-MIDDLE-"
#define END_WORD		"-END-"
#define END_POSTAG		"-END-"

#define NULL_LABEL		"-NULL-"

#define SENT_SPTOKEN	"/"
#define SENT_WORD(X)	(std::get<0>(X))
#define SENT_POSTAG(X)	(std::get<1>(X))

#define TREENODE_WORD(X)			(std::get<0>(std::get<0>(X)))
#define TREENODE_POSTAG(X)			(std::get<1>(std::get<0>(X)))
#define TREENODE_POSTAGGEDWORD(X)	(std::get<0>(X))
#define TREENODE_HEAD(X)			(std::get<1>(X))
#define TREENODE_LABEL(X)			(std::get<2>(X))

#define ENCODE_L2R(X)			((X) << 1)
#define ENCODE_R2L(X)			(((X) << 1) + 1)
#define ENCODE_2ND_L2R(X,Y)		ENCODE_L2R(((X) << MAX_SENTENCE_BITS) | (Y))
#define ENCODE_2ND_R2L(X,Y)		ENCODE_R2L(((X) << MAX_SENTENCE_BITS) | (Y))

#define ENCODE_EMPTY(X,T)		(((T) << MAX_SENTENCE_BITS) | (X))
#define DECODE_EMPTY_POS(X)		((X) & ((1 << MAX_SENTENCE_BITS) - 1))
#define DECODE_EMPTY_TAG(X)		((X) >> MAX_SENTENCE_BITS)

#define GRAPH_LEFT	-1
#define GRAPH_RIGHT 1

typedef int gtype;

// features type
typedef unsigned int Unsigned;
typedef int Int;
typedef gtype Word;
typedef gtype POSTag;
typedef gtype SuperTag;

typedef BiGram<gtype> TwoInts;
typedef BiGram<gtype> TwoWords;
typedef BiGram<gtype> POSTagSet2;
typedef BiGram<gtype> WordPOSTag;
typedef QuarGram<unsigned int> WordSetOfDepLabels;
typedef QuarGram<unsigned int> POSTagSetOfDepLabels;

typedef TriGram<gtype> ThreeInts;
typedef TriGram<gtype> ThreeWords;
typedef TriGram<gtype> POSTagSet3;
typedef TriGram<gtype> WordIntInt;
typedef TriGram<gtype> POSTagIntInt;
typedef TriGram<gtype> WordWordPOSTag;
typedef TriGram<gtype> WordPOSTagPOSTag;

typedef QuarGram<gtype> FourInts;
typedef QuarGram<gtype> POSTagSet4;
typedef QuarGram<gtype> ThreeWordsInt;
typedef QuarGram<gtype> WordWordPOSTagPOSTag;
typedef QuarGram<gtype> WordWordPOSTagInt;

typedef BiGram<gtype> WordInt;
typedef BiGram<gtype> POSTagInt;

typedef TriGram<gtype> TwoWordsInt;
typedef TriGram<gtype> POSTagSet2Int;
typedef TriGram<gtype> WordPOSTagInt;
typedef TriGram<Unsigned> ThreeUnsignedS;

typedef QuarGram<gtype> POSTagSet3Int;
typedef QuarGram<gtype> TwoWordsIntInt;
typedef QuarGram<gtype> POSTagSet2IntInt;
typedef QuarGram<gtype> WordWordPOSTagInt;
typedef QuarGram<gtype> WordPOSTagPOSTagInt;

typedef QuinGram<gtype> POSTagSet4Int;
typedef QuinGram<gtype> WordWordPOSTagPOSTagInt;
typedef QuinGram<gtype> WordPOSTagPOSTagPOSTagInt;

typedef HexaGram<gtype> POSTagSet5Int;
typedef HexaGram<gtype> WordPOSTagPOSTagPOSTagPOSTagInt;

// features map define
typedef PackedScoreMap<WordInt> WordIntMap;
typedef PackedScoreMap<POSTagInt> POSTagIntMap;

typedef PackedScoreMap<TwoWordsInt> TwoWordsIntMap;
typedef PackedScoreMap<POSTagSet2Int> POSTagSet2IntMap;
typedef PackedScoreMap<WordPOSTagInt> WordPOSTagIntMap;

typedef PackedScoreMap<POSTagSet3Int> POSTagSet3IntMap;
typedef PackedScoreMap<WordWordPOSTagInt> WordWordPOSTagIntMap;
typedef PackedScoreMap<WordPOSTagPOSTagInt> WordPOSTagPOSTagIntMap;

typedef PackedScoreMap<POSTagSet4Int> POSTagSet4IntMap;
typedef PackedScoreMap<WordWordPOSTagPOSTagInt> WordWordPOSTagPOSTagIntMap;
typedef PackedScoreMap<WordPOSTagPOSTagPOSTagInt> WordPOSTagPOSTagPOSTagIntMap;

// data structure
typedef std::tuple<ttoken, ttoken> POSTaggedWord;
typedef std::tuple<POSTaggedWord, int, ttoken> DependencyTreeNode;
typedef std::pair<DependencyTreeNode, ttoken> DependencyTaggedTreeNode;

typedef std::vector<POSTaggedWord> Sentence;
typedef std::vector<DependencyTreeNode> DependencyTree;
typedef std::vector<DependencyTaggedTreeNode> DependencyTaggedTree;

typedef std::unordered_map<int, std::vector<int>> SuperTagCandidates;

int encodeLinkDistance(const int & st, const int & n0);
int encodeEmptyDistance(const int & hi, const int & di);
int encodeLinkDistanceOrDirection(const int & hi, const int & di, bool dir);
std::string nCharPrev(const Sentence & sent, int index, int n);
std::string nCharNext(const Sentence & sent, int index, int n);

std::istream & operator>>(std::istream & input, Sentence & sentence);
std::istream & operator>>(std::istream & input, DependencyTree & tree);
std::istream & operator>>(std::istream & input, DependencyTaggedTree & tree);

std::ostream & operator<<(std::ostream & output, const Sentence & sentence);
std::ostream & operator<<(std::ostream & output, const DependencyTree & tree);
std::ostream & operator<<(std::ostream & output, const DependencyTaggedTree & tree);

void nBackSpace(const std::string & str);
bool hasNonProjectiveTree(std::set<std::pair<int, int>> goldArcs, int len);
DependencyTree emptyToNonEmpty(const DependencyTree & tree);

#endif
