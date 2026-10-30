#include <stdio.h>
#include <string.h>
#include "tcg/tcg.h"
#include "tcg/card_ids.h"
#include "tcg_structure_decks.h"
extern const struct TcgDeckDefinition gTcgSpecialDecks[];
extern const unsigned gTcgSpecialDeckCount;
extern const uint16_t gTcgSpecialPromos[];
int Tcg_IsHolo(unsigned card);
unsigned Tcg_PointCost(unsigned card);
unsigned Tcg_ShopPrice(unsigned card);
static void S(const char *s){putchar('"');for(;s&&*s;s++){if(*s=='"'||*s=='\\')putchar('\\');putchar(*s);}putchar('"');}
static void Cards(const uint16_t *c,unsigned n){unsigned i;putchar('[');for(i=0;i<n;i++)printf("%s%u",i?",":"",c[i]);putchar(']');}
int main(void){
  unsigned i,p;
  printf("{\"cards\":[");
  for(i=1;i<=TCG_CARD_COUNT;i++){const struct TcgCard*c=Tcg_Card(i);
    printf("%s{\"id\":%u,\"name\":",i>1?",":"",i);S(c->name);
    printf(",\"kind\":%u,\"type\":%u,\"set\":%u,\"set2\":%u,\"rarity\":%u,\"holo\":%d,\"stage\":%u,\"level\":%u,\"packs\":[",c->kind,c->type,c->set,c->set2,c->rarity,Tcg_IsHolo(i),c->stage,c->level);
    { int f=1; for(p=0;p<TCG_PACK_TYPES;p++) if(Tcg_InPack(i,p) && c->kind!=TCG_ENERGY && c->rarity<3){printf("%s%u",f?"":",",p);f=0;} }
    printf("],\"cp\":%u,\"money\":%u}",Tcg_PointCost(i),Tcg_ShopPrice(i));}
  printf("],\"decks\":[");
  for(i=0;i<gTcgDeckDefinitionCount;i++){printf("%s{\"name\":",i?",":"");S(gTcgDeckDefinitions[i].name);printf(",\"cards\":");Cards(gTcgDeckDefinitions[i].cards,TCG_DECK_SIZE);putchar('}');}
  printf("],\"special\":[");
  for(i=0;i<gTcgSpecialDeckCount;i++){printf("%s{\"name\":",i?",":"");S(gTcgSpecialDecks[i].name);printf(",\"promo\":%u,\"cards\":",gTcgSpecialPromos[i]);Cards(gTcgSpecialDecks[i].cards,TCG_DECK_SIZE);putchar('}');}
  printf("],\"structure\":[");
  for(i=0;i<sizeof(sStructureDecks)/sizeof(sStructureDecks[0]);i++){printf("%s{\"name\":",i?",":"");S(sStructureDecks[i].name);printf(",\"set\":");S(sStructureDecks[i].set);printf(",\"price\":%u,\"cards\":",sStructureDecks[i].price);Cards(sStructureDecks[i].cards,TCG_DECK_SIZE);putchar('}');}
  printf("]}\n");return 0;}
