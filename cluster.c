/**
 * Kostra programu pro 2. projekt IZP 2022/23
 *
 * Jednoducha shlukova analyza: 2D nejblizsi soused.
 * Single linkage
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h> // sqrtf
#include <limits.h> // INT_MAX
#include <string.h>
#include <ctype.h>

/*****************************************************************
 * Ladici makra. Vypnout jejich efekt lze definici makra
 * NDEBUG, napr.:
 *   a) pri prekladu argumentem prekladaci -DNDEBUG
 *   b) v souboru (na radek pred #include <assert.h>
 *      #define NDEBUG
 */
#ifdef NDEBUG
#define debug(s)
#define dfmt(s, ...)
#define dint(i)
#define dfloat(f)
#else

// vypise ladici retezec
#define debug(s) printf("- %s\n", s)

// vypise formatovany ladici vystup - pouziti podobne jako printf
#define dfmt(s, ...) printf(" - "__FILE__":%u: "s"\n",__LINE__,__VA_ARGS__)

// vypise ladici informaci o promenne - pouziti dint(identifikator_promenne)
#define dint(i) printf(" - " __FILE__ ":%u: " #i " = %d\n", __LINE__, i)

// vypise ladici informaci o promenne typu float - pouziti
// dfloat(identifikator_promenne)
#define dfloat(f) printf(" - " __FILE__ ":%u: " #f " = %g\n", __LINE__, f)

#endif

/*****************************************************************
 * Deklarace potrebnych datovych typu:
 *
 * TYTO DEKLARACE NEMENTE
 *
 *   struct obj_t - struktura objektu: identifikator a souradnice
 *   struct cluster_t - shluk objektu:
 *      pocet objektu ve shluku,
 *      kapacita shluku (pocet objektu, pro ktere je rezervovano
 *          misto v poli),
 *      ukazatel na pole shluku.
 */

struct obj_t {
    int id;
    float x;
    float y;
};

struct cluster_t {
    int size;
    int capacity;
    struct obj_t *obj;
};

/*****************************************************************
 * Deklarace potrebnych funkci.
 *
 * PROTOTYPY FUNKCI NEMENTE
 *
 * IMPLEMENTUJTE POUZE FUNKCE NA MISTECH OZNACENYCH 'TODO'
 *
 */

/*
 Inicializace shluku 'c'. Alokuje pamet pro cap objektu (kapacitu).
 Ukazatel NULL u pole objektu znamena kapacitu 0.
*/
void init_cluster(struct cluster_t *c, int cap)
{
    assert(c != NULL);
    assert(cap >= 0);

    c->size = 0;
    c->capacity = cap;
    c->obj = malloc(c->capacity * sizeof(struct obj_t)); // alokace pameti pro objekt
    if(c->obj == NULL){ // pokud je ukazatel NULL u pole objektu, tak to znamena, ze nastavi kapacitu na 0
        c->capacity = 0;           
    }
}

/*
 Odstraneni vsech objektu shluku a inicializace na prazdny shluk.
 */
void clear_cluster(struct cluster_t *c)
{ 
    assert(c);  
    // vynulovani hodnot a uvolneni objektu
    c->size = 0; 
    c->capacity = 0;
    free(c->obj);  
}

/// Chunk of cluster objects. Value recommended for reallocation.
const int CLUSTER_CHUNK = 10;

/*
 Zmena kapacity shluku 'c' na kapacitu 'new_cap'.
 */
struct cluster_t *resize_cluster(struct cluster_t *c, int new_cap)
{
    // TUTO FUNKCI NEMENTE
    assert(c);
    assert(c->capacity >= 0);
    assert(new_cap >= 0);

    if (c->capacity >= new_cap)
        return c;

    size_t size = sizeof(struct obj_t) * new_cap;

    void *arr = realloc(c->obj, size);
    if (arr == NULL)
        return NULL;

    c->obj = (struct obj_t*)arr;
    c->capacity = new_cap;
    return c;
}

/*
 Prida objekt 'obj' na konec shluku 'c'. Rozsiri shluk, pokud se do nej objekt
 nevejde.
 */
void append_cluster(struct cluster_t *c, struct obj_t obj)
{
    assert(c);
   
    c->size++; 
    if(c->size >= c->capacity){ // pokud je velikost clusteru vetsi, nez jeho kapacita, tak musime jeho kapacitu zvetsit
        resize_cluster(c, c->capacity + CLUSTER_CHUNK); // zvetseni kapacity
    }
    
    c->obj[c->size - 1] = obj; // ulozi obj na posledni pozici v clusteru

}

/*
 Seradi objekty ve shluku 'c' vzestupne podle jejich identifikacniho cisla.
 */
void sort_cluster(struct cluster_t *c);

/*
 Do shluku 'c1' prida objekty 'c2'. Shluk 'c1' bude v pripade nutnosti rozsiren.
 Objekty ve shluku 'c1' budou serazeny vzestupne podle identifikacniho cisla.
 Shluk 'c2' bude nezmenen.
 */
void merge_clusters(struct cluster_t *c1, struct cluster_t *c2)
{
    assert(c1 != NULL);
    assert(c2 != NULL);

    if(c1->capacity < c1->size + c2->size){ // pokud je kapacita prvniho clusteru mensi, nez soucet velikosti obou clusteru, musi se zvetsit
        resize_cluster(c1, c1->size + c2->size);
    }
    for(int i = 0; i < c2->size; i++){ // cyklus, ktery pridava objekty z druheho clusteru na konec prvniho
        append_cluster(c1, c2->obj[i]);        
    }
    sort_cluster(c1); // serazeni clusteru
}

/**********************************************************************/
/* Prace s polem shluku */

/*
 Odstrani shluk z pole shluku 'carr'. Pole shluku obsahuje 'narr' polozek
 (shluku). Shluk pro odstraneni se nachazi na indexu 'idx'. Funkce vraci novy
 pocet shluku v poli.
*/
int remove_cluster(struct cluster_t *carr, int narr, int idx)
{
    assert(idx < narr);
    assert(narr > 0);

    clear_cluster(&carr[idx]); // odstrani vsechny objekty daneho clusteru

    for(int i = idx + 1; i < narr; i++){ // cyklus, ktery posouva vsechny prvky napravo od odstraneneho o jeden doleva
        carr[i-1] = carr[i];
    }
    narr--;
    return narr; // vraci novy pocet v poli clusteru
}

/*
 Pocita Euklidovskou vzdalenost mezi dvema objekty.
 */
float obj_distance(struct obj_t *o1, struct obj_t *o2)
{
    assert(o1 != NULL);
    assert(o2 != NULL);

    if(o1 == NULL || o2 == NULL){
        fprintf(stderr, "Empty objects...");
        return -1;
    }
    float distance = sqrtf(pow((o1->x - o2->x), 2) + pow((o1->y - o2->y), 2)); // vzorec Euklidovy vzdalenosti pro dve dimenze
    return distance;
}

/*
 Pocita vzdalenost dvou shluku.
*/
float cluster_distance(struct cluster_t *c1, struct cluster_t *c2)
{
	assert(c1 != NULL);
	assert(c1->size > 0);
	assert(c2 != NULL);
	assert(c2->size > 0);

    if(c1 == NULL || c2 == NULL || c1->size <= 0 || c2->size <= 0){ // kontrola vstupu
        fprintf(stderr, "ERROR in cluster_distance");
        return -1;
    }
    float min = obj_distance(&c1->obj[0], &c2->obj[0]); // nastavi minimum na vzdalenost nultych objektu clusteru
    for(int i = 0; i < c1->size; i++){
        for(int j = 0; j < c2->size; j++){
            if(obj_distance(&c1->obj[i], &c2->obj[j]) < min){ // pokud pocitana vzdalenost je mensi, nez momentalni minimum, tak zmeni hodnotu minima na tuto vzdalenost
                min = obj_distance(&c1->obj[i], &c2->obj[j]);
            }
        }
    }
    return min;
}

/*
 Funkce najde dva nejblizsi shluky. V poli shluku 'carr' o velikosti 'narr'
 hleda dva nejblizsi shluky. Nalezene shluky identifikuje jejich indexy v poli
 'carr'. Funkce nalezene shluky (indexy do pole 'carr') uklada do pameti na
 adresu 'c1' resp. 'c2'.
*/
void find_neighbours(struct cluster_t *carr, int narr, int *c1, int *c2)
{
	assert(narr > 0);
	assert(carr);
	if (narr <= 0 || carr == NULL) // kontrola vstupu
	{
		fprintf(stderr, "ERROR in find_neighbours");
		*c1 = -1; // uklada chybu do pointeru
	}
    else{        
        float min = INT_MAX; //nastavi minimum jako nejvetsi hodnotu
        for (int i = 0; i < narr; i++){
            for (int j = i + 1; j < narr; j++){
                if (cluster_distance(&carr[i],&carr[j]) == -1) //konrola, zda nastala chyba ve fci cluster_distance
                {
                    *c1 = -1; // uklada chybu do pointeru
                    return;
                }
                else{                
                    if (cluster_distance(&carr[i],&carr[j]) < min){ // urci, zda je vzdalenost mensi, nez minimum
                        min = cluster_distance(&carr[i],&carr[j]); // nastavi minimum na vzdalenost aktualne pocitanych clusteru
                        *c1 = i; // ulozi do pointeru index i
                        *c2 = j; // ulozi do pointeru index j
                    }
                }
            }
        }
    }
}
/*
 Pomocna funkce pro razeni shluku
*/ 
static int obj_sort_compar(const void *a, const void *b)
{
    // TUTO FUNKCI NEMENTE
    const struct obj_t *o1 = (const struct obj_t *)a;
    const struct obj_t *o2 = (const struct obj_t *)b;
    if (o1->id < o2->id) return -1;
    if (o1->id > o2->id) return 1;
    return 0;
}

/*
 Razeni objektu ve shluku vzestupne podle jejich identifikatoru.
*/
void sort_cluster(struct cluster_t *c)
{
    // TUTO FUNKCI NEMENTE
    qsort(c->obj, c->size, sizeof(struct obj_t), &obj_sort_compar);
}

/*
 Tisk shluku 'c' na stdout.
*/
void print_cluster(struct cluster_t *c)
{
    // TUTO FUNKCI NEMENTE
    for (int i = 0; i < c->size; i++)
    {
        if (i) putchar(' ');
        printf("%d[%g,%g]", c->obj[i].id, c->obj[i].x, c->obj[i].y);
    }
    putchar('\n');
}

// fce, ktera vraci pocet z prvniho radku a kontroluje, jestli je kladne cislo a jestli u nej neni neco navic
int get_count(char *str, int *count){
    int num;
    if(1 == sscanf(str, "count=%d", &num)){ //ignoruje znaky krome cisel a cisla ulozi do promenne num
        if(num < 0){ // vraci chybu, pokud je pocet radku/num zaporny
            return -1;
        }
        else{
            *count = num;
            return *count; // vraci hodnotu poctu radku         
        }
    }
    else{ // vraci chybu, pokud v prvnim radku neco prebyva
        fprintf(stderr, "ERROR, couldnt find count in get_count");
        return -1;
    }
}

// fce, ktera kontroluje jedinecnost ID
int id_check(int id, struct cluster_t *cluster, int radek)
{
	for (int i = 0; i < radek; i++)
		if (cluster[i].obj->id == id) // vraci chybu, pokud ID neni jedinecne
		{	
			return -1;
		}
	return 0;
}

// fce, ktera kontroluje spravnost hodnot objektu
int object_check(float num){
    if(num < 0 || num > 1000){ // vraci chybu, pokud hodnota neni z rozsahu
        return -1;
    }
    else if(num - (int)num != 0){ // vraci chybu, pokud hodnota neni celociselna
        return -1; 
    }
    else return 0;
}

 // fce, ktera odstrani vsechny clustery a uvolni pamet
void remove_all_clusters(struct cluster_t **arr, int cluster_count){ 
    for(int i = 0; i < cluster_count; i++){
        clear_cluster(&(*arr)[i]);
    }
    free(*arr);
}


/*
 Ze souboru 'filename' nacte objekty. Pro kazdy objekt vytvori shluk a ulozi
 jej do pole shluku. Alokuje prostor pro pole vsech shluku a ukazatel na prvni
 polozku pole (ukalazatel na prvni shluk v alokovanem poli) ulozi do pameti,
 kam se odkazuje parametr 'arr'. Funkce vraci pocet nactenych objektu (shluku).
 V pripade nejake chyby uklada do pameti, kam se odkazuje 'arr', hodnotu NULL.
*/
int load_clusters(char *filename, struct cluster_t **arr)
{
    assert(arr != NULL);
    if(arr == NULL){ // pokud je arr == NULL, tak vyhodi chybu
        fprintf(stderr, "ERROR in load_clusters...");
        return -1;
    }

    FILE *f; 
    if((f = fopen(filename, "r")) == NULL){ // pokud se soubor nepodari otevrit, vyhodi chybu
        fprintf(stderr, "ERROR, couldn't open file...");
        return -1;
    }
    int count = 0;
    int max_length = 30;
    char str[31];
    char extra;

    if(fgets(str, max_length, f) != NULL){// nacte prvni radek do promenne str
        if(get_count(str, &count) == -1){ // kontrola prvniho radku s "count=", zda neobsahuje znaky, ktere nema
            fprintf(stderr, "Wrong count...");
            fclose(f); // pokud se vyskytne chyba, soubor se zavre
            return -1;
        }
    }    
    else{ // pokud ze souboru nic nejde nacist, vyhodi chybu
        fprintf(stderr, "Count line doesn't exist..."); 
        fclose(f); // pokud se vyskytne chyba, soubor se zavre
        return -1;
    }
    *arr = malloc(count * sizeof(struct cluster_t)); //alokace pameti pro pole clusteru
    if(arr == NULL){ // pokud se nepodari alokovat pamet, vyhodi chybu
        fprintf(stderr, "ERROR, memory wasn't allocated properly...");
        fclose(f); // pokud se vyskytne chyba, soubor se zavre
        return -1;
    }
    for(int i = 0; i < count; i++){
        init_cluster(&(*arr)[i], 1); // inicializace vsech clusteru/radku na kapacitu 1
    }

	struct obj_t obj;
    for (int i = 0; i < count; i++){ // cyklus pro nacitani vsech radku
        if (fgets(str, max_length, f) != NULL){ //kontroluje existenci dalsiho radku
            if (sscanf(str,"%d %f %f %c",&obj.id ,&obj.x ,&obj.y, &extra) != 3 || (object_check(obj.x) == -1) || (object_check(obj.y) == -1)){ // kontrola, zda je radek zadany spravne (v rozsahu, cela cisla, nic navic)
                fprintf(stderr, "Line isn't correct...");
                fclose(f); // pokud se vyskytne chyba, tak se soubor zavre, vsechno odstrani a odalokuje
                remove_all_clusters(arr, count); 
                return -1;
            }
            if (id_check(obj.id,*arr,i) == -1){ // kontrola, zda je kazde id jedinecne
                fprintf(stderr, "ID isn't unique...");
                fclose(f); // pokud se vyskytne chyba, tak se soubor zavre, vsechno odstrani a odalokuje
                remove_all_clusters(arr, count);
                return -1;   
            }
            append_cluster(&(*arr)[i], obj); // pokud je vsechno spravne, tak priradime objekt do clusteru     
        }
        else{
            fprintf(stderr, "Couldn't load line..."); // pokud ze souboru nejde nic nacist, vyhodi chybu
            fclose(f); // pokud se vyskytne chyba, tak se soubor zavre, vsechno odstrani a odalokuje
            remove_all_clusters(arr, count);
            return -1;
        }
    }
    fclose(f); // uzavreni souboru
    return count; // fce vraci pocet radku/clusteru v souboru
}

/*
 Tisk pole shluku. Parametr 'carr' je ukazatel na prvni polozku (shluk).
 Tiskne se prvnich 'narr' shluku.
*/
void print_clusters(struct cluster_t *carr, int narr)
{
    // TUTO FUNKCI NEMENTE
    printf("Clusters:\n");
    for (int i = 0; i < narr; i++)
    {
        printf("cluster %d: ", i);
        print_cluster(&carr[i]);
    }
}

 // fce, ktera kontroluje, jestli je dany retezec tvoreny pouze cislicemi
int digits_only(char *str){
    while(*str){
        if(isdigit(*str++) == 0) return -1;
    }
    return 0;
}

int main(int argc, char *argv[])
{
	struct cluster_t *clusters;
    char filename[20];
    int final_count = 0;

    if(argc < 2){ // kontrola, jestli neni malo argumentu
        fprintf(stderr, "ERROR, not enough args...");
        return -1;
    }
    else if(argc > 3){ // kontrola, jestli neni moc argumentu
        fprintf(stderr, "ERROR, too many args...");
        return -1;
    }
    else{
        strcpy(filename, argv[1]); // zkopirovani nazvu souboru do promenne filename
        if(argc == 3){ // pokud jsou argumenty 3, tak ma cenu hledat pocet finalnich clusteru jako argument   
            if(digits_only(argv[2]) == -1){ // kontrola argumentu, jestli se je pouze ciselny
                fprintf(stderr, "ERROR, only numbers in second argument allowed...");
                return -1;
            }
            else{
                final_count = atoi(argv[2]); // ziskani poctu finalnich clusteru
                if(final_count <= 0){ // kontrola, jestli neni pozadovany pocet zaporny
                    fprintf(stderr, "ERROR, only positive numbers in second argument allowed...");
                    return -1;
                }
            }
        }
        else{
            final_count = 1; // pokud jsou jen 2 argumenty, tak automaticky nastavi pozadovany pocet clusteru na 1
        }
        int count = load_clusters(filename, &clusters); // zjisti pocet radku/clusteru v souboru
        if(count == -1){ // pokud funkce vrati chybu, tak konci program
            return -1;
        }
        if(count >= final_count){
            for(int i = count; i > final_count; i--){ // spojuje clustery do doby, nez dostane pozadovany pocet clusteru
                int c1, c2;
                find_neighbours(clusters, count, &c1, &c2); // hleda dva nejblizsi clustery a vraci indexy techto clusteru jako c1, c2
                if(c1 == -1){ // pokud se pri hledani sousedu stala chyba, ukonci program
                    return -1;
                }
                merge_clusters(&clusters[c1], &clusters[c2]); // spoji dva nejblizsi clustery
                count = remove_cluster(clusters, count, c2); // zmeni pocet na novy po odstraneni clusteru
                if(count == -1){
                    return -1;
                }
            }

            print_clusters(clusters, count); // vypise vsechny clustery            
        }
        else{
            fprintf(stderr, "Final count is bigger than amount of lines in entered file...."); // pokud je cileny pocet clusteru vetsi, nez puvodni pocet clusteru v souboru, tak vyhodi chybu
            return -1;
        }
        remove_all_clusters(&clusters, count); // fce na odstraneni vsech clusteru a uvolneni pameti
    }
    return 0;
}
