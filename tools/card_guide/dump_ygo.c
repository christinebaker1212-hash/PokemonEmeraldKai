#include <stdio.h>
#include "ygo/ygo.h"
#include "ygo/collection.h"
int Ygo_CardReady(unsigned id);
unsigned Ygo_PointCost(unsigned card);
unsigned Ygo_MoneyPrice(unsigned card);
static void S(const char *s){putchar('"');for(;s&&*s;s++){if(*s=='"'||*s=='\\')putchar('\\');if((unsigned char)*s>=32)putchar(*s);}putchar('"');}
int main(void){
  unsigned i,k;
  Ygo_CardReady(1); fflush(stdout);
  printf("\n{\"cards\":[");
  for(i=1;i<=YGO_CARD_COUNT;i++){const struct YgoCard*c=Ygo_Card(i);
    printf("%s{\"id\":%u,\"name\":",i>1?",":"",i);S(c->name);
    printf(",\"frame\":%u,\"rarity\":%u,\"ready\":%d,\"cp\":%u,\"money\":%u,\"level\":%u,\"atk\":%u}",c->frame,c->rarity,Ygo_CardReady(i),Ygo_PointCost(i),Ygo_MoneyPrice(i),c->level,c->atk);}
  printf("],\"sets\":[");
  for(i=0;i<YGO_SET_COUNT;i++){const struct YgoSet*s=&gYgoSets[i];
    printf("%s{\"code\":",i?",":"");S(s->code);printf(",\"name\":");S(s->name);printf(",\"starter\":%u,\"cards\":[",s->starter);
    for(k=0;k<s->count;k++)printf("%s[%u,%u,%u]",k?",":"",s->cards[k].card,s->cards[k].rarity,s->cards[k].copies);
    printf("]}");}
  printf("]}\n");return 0;}
