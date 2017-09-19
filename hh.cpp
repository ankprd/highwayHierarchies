#include <cstdio>
#include <queue>
#include <vector>
#include <map>
#include <ctime>

using namespace std;

struct Way
{
    int arr;
    int poids;

    Way() {}
    Way(int utArr, int utPoids) : arr(utArr), poids(utPoids) {}
};

struct Route
{
    int dep;
    int arr;

    Route() {}
    Route(int utDep, int utArr) : dep(utDep), arr(utArr) {}

    bool operator < (const Route &other) const
    {
        if(dep == other.dep)
            return arr > other.arr;
        return dep > other.dep;
    }
};

struct SNoeud
{
    int idN;
    int distADep;
    int idAj;//pour assurer le caractere FIFO de la file de priorite en cas d egalite

    SNoeud (){}
    SNoeud (int utIdN, int utDAD, int utidAj) : idN(utIdN), distADep(utDAD), idAj(utidAj){}
    SNoeud (const SNoeud& utSNoeud) : idN(utSNoeud.idN), distADep(utSNoeud.distADep), idAj(utSNoeud.idAj) {}

    bool operator < (const SNoeud &other) const
    {
        if(distADep == other.distADep)
            return idAj > other.idAj;
        return distADep > other.distADep;
    }
};

struct CaseTab
{
    int date; //le tableau va etre utilise plusieurs fois sans reinitialisation
    int val;
    bool actif;
    bool dejaTraite; //dans la construction de l'arbre de plus court chemin
};

struct Fils
{
    int date;
    vector <int> enfants;
};
////////////////////////////////////////////////////////////////////////////////////////////
const int NB_NOEUDS = 204477;
const int NB_NIVEAUX = 20;
const int h = 125;

map<Route, int> route;

FILE *fout;
FILE *fout2;

int date = 1;//pas 0 car tous les tableaux sont initialises a 0 car declares en global

vector <Way> chemins[NB_NOEUDS * (NB_NIVEAUX + 1)];
CaseTab dist[NB_NOEUDS];
map<Route, vector <int> > cheminComplet;//apres compaction, contient la liste des idR auxquels le chemin correspond en realite

vector <int> hVois[NB_NOEUDS];
Fils fils[NB_NOEUDS];
int pere[NB_NOEUDS];//pas besoin de dates car si on est arrives au pere, on est passe par le fils
int ancetre[NB_NOEUDS]; //correspond a s1
///////////////////////////////////////////////////////////////////////////////////////////////

void litGraphe()
{
    FILE *fin  = fopen ("entreeIntermediaire.txt", "r");
    fout = fopen ("listeChems.txt", "w");
    fout2 = fopen ("nbChems.txt", "w");
    int nbChemins;
    fscanf(fin, "%d", &nbChemins);
    for(int curChemin = 0; curChemin < nbChemins; curChemin++)
    {
        int idR, deb, arr, poids;
        fscanf(fin, "%d%d%d%d", &idR, &poids, &deb, &arr);
        //printf("lect ok");

        vector <int> chem;
        chem.push_back(idR);
        cheminComplet[Route(deb, arr)] = chem;
        cheminComplet[Route(arr, deb)] = chem;
        //printf("%d %d %d %d\n", idR, poids, deb, arr);
        chemins[deb].push_back(Way(arr, poids));
        chemins[arr].push_back(Way(deb, poids));
        //printf("lalala %d", idR);
    }
    //fprintf(fout, "test");
}

void compacteChemins(int idNiv)
{
    int correctif = idNiv * NB_NOEUDS;
    for(int curN = 0; curN < NB_NOEUDS; curN++)
    {
        if(chemins[curN + correctif].size() == 2)
        {
            int dep = chemins[curN + correctif][0].arr;
            int arr = chemins[curN + correctif][1].arr;
            int poids = chemins[curN + correctif][0].poids + chemins[curN + correctif][1].poids;

            chemins[curN + correctif].clear();

            //printf("\n\nnoeud a mod : %d, dep %d,arr %d, poids %d\n", curN, dep, arr, poids);

            //on verifie si il y avait deja un chemin de dep a arr. Si le nouveau est plus court, on enleve l'ancien, si l'ancien est plus court, on n'ajoute pas le nouveau
            int posDep = -1;
            for(int i = 0; i < chemins[arr].size(); i++)
                if(chemins[arr][i].arr == dep)
                {
                    posDep = i;
                    break;
                }
            //printf("posDep : %d\n", posDep);
            bool aRajouter = false;
            if (posDep != -1 && chemins[arr][posDep].poids > poids)
            {
                //printf("on enleve l' ancien chemin et %d\n", chemins[arr][posDep].arr);
                aRajouter = true;
                chemins[arr].erase(chemins[arr].begin() + posDep);
                int pos = 0;
                while(chemins[dep][pos].arr != arr)
                    pos++;
                chemins[dep].erase(chemins[dep].begin() + pos);
            }
            else if (posDep == -1)
                aRajouter = true;

            //on supprime le chemin de arr a curN et celui de dep a curN
            int pos = 0;
            while(chemins[arr][pos].arr != curN + correctif)
                pos++;
            chemins[arr].erase(chemins[arr].begin() + pos);
            /*printf("chemins avant pour arr:\n");
            for(int i = 0; i < chemins[arr].size(); i++)
                printf("%d ", chemins[arr][i].arr);*/
            pos = 0;
            while(chemins[dep][pos].arr != curN + correctif)
                pos++;
            /*printf("\nchemins avant pour dep:\n");
            for(int i = 0; i < chemins[dep].size(); i++)
                printf("%d ", chemins[dep][i].arr);*/
            chemins[dep].erase(chemins[dep].begin() + pos);

            if(aRajouter)//si on doit rajouter dep -> arr (il n'exsite pas deja ou le nouveau est de poids plus faible donc l'ancien a ete enleve
            {
                chemins[arr].push_back(Way(dep, poids));
                chemins[dep].push_back(Way(arr, poids));
            }
            /*printf("\napres pour arr:\n");
            for(int i = 0; i < chemins[arr].size(); i++)
                printf("%d ", chemins[arr][i].arr);*/
            /*printf("\napres pour dep:\n");
            for(int i = 0; i < chemins[dep].size(); i++)
                printf("%d ", chemins[dep][i].arr);*/

            //on met à jour cheminComplet
            vector <int> cheminCompl = cheminComplet[Route(dep, curN + correctif)];
            vector <int> aAj = cheminComplet[Route(curN + correctif, arr)];
            for(unsigned int i = 0; i < aAj.size(); i++)
                cheminCompl.push_back(aAj[i]);
            cheminComplet.erase(Route(dep, curN + correctif));
            cheminComplet.erase(Route(curN + correctif, dep));
            cheminComplet.erase(Route(arr, curN + correctif));
            cheminComplet.erase(Route(curN + correctif, arr));

            if(aRajouter)
            {
                cheminComplet[Route(dep, arr)] = cheminCompl;
                cheminComplet[Route(arr, dep)] = cheminCompl;
            }
        }
    }
}

int donneDate()
{
    //permet de reutiliser plusieurs fois un tableau sans le reinitialiser
    date++;
    return (date - 1);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vector <int> dHEtHVois(int source, int idNiv)//idNiv correspond au niveau auquel appartiennent les chemins etudies => un niveau en dessous de celui qu'on construit
                                             //source est % NB_NOEUDS
{
    //Retourne un vect d'entiers: les noeuds du Hvois + en derniere position, dH
    //le hVois depend du niveau, mais dans le vect qui est retourne, on a les noeuds % NB_NOEUDS
    //mais dans l'algo de plus court chemin, on travaille avec l'indice reel des noeuds (pas % NB_NOEUDS)
    vector <int> voisinage;
    int correctif = idNiv * NB_NOEUDS;
    int curRg = -1; //apres incrementation, vaut le rang dans l'algo de dijkstra du noeud en cours de traitement dans le while
    int dH = 0;
    int dateAppelFct = donneDate();
    priority_queue <SNoeud> nAV; //noeudsAVoir
    int dateAjNoeud = 0;

    nAV.push(SNoeud(source + correctif, 0, dateAjNoeud));
    dateAjNoeud++;
    dist[source].val = 0;
    dist[source].date = dateAppelFct;
    while(!nAV.empty())
    {
        curRg++;
        SNoeud curN = nAV.top();
        nAV.pop();
        //printf("curN : %d curRg : %d distASource : %d\n", curN.idN, curRg, curN.distADep);

        //A-t-on finit de parcourir le h vois ?
        if (curRg == h)
            dH = curN.distADep;
        if(curRg > h && curN.distADep > dH)
            break;

        //Parcours des voisins
        voisinage.push_back(curN.idN % NB_NOEUDS);
        for(unsigned int k = 0; k < chemins[curN.idN].size(); k++)
        {
            int nouvDist =  chemins[curN.idN][k].poids + curN.distADep;
            int idVois = chemins[curN.idN][k].arr % NB_NOEUDS;
            if(dist[idVois].date != dateAppelFct || dist[idVois].val > nouvDist)
            {
                nAV.push(SNoeud(chemins[curN.idN][k].arr, nouvDist, dateAjNoeud));
                dateAjNoeud++;
                dist[idVois].val = nouvDist;
                dist[idVois].date = dateAppelFct;
            }
        }
    }
    voisinage.push_back(dH);
    return voisinage;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int longChem(int dep, int arr)//dep et arr ne sont pas % NB_NOEUDS
{
    //printf("        dep : %d, arr : %d\n", dep,  arr);
    for(unsigned int k = 0; k < chemins[dep].size(); k++)
        if(chemins[dep][k].arr == arr)
            return chemins[dep][k].poids;
    fprintf(fout, "%d %d AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAHHHHHHHHHHHHHHH\n", dep % NB_NOEUDS, arr % NB_NOEUDS);//devrait jamais arriver,
    return 0;
}

bool dejaAjChem(int dep, int arr)
{
    for(unsigned int k = 0; k < chemins[dep].size(); k++)
        if(chemins[dep][k].arr == arr)
            return true;
    return false;
}

bool estDedans(int elt, vector <int> liste)//on regarde pas la derniere case de liste car elle correspond a dH
{
    for(unsigned int k = 0; k < liste.size() - 1; k++)
        if(elt == liste[k])
            return true;
    return false;
}

bool estActifAtt(int noeud)//noeud est % NB_NOEUDS
{
    //Donne l'etat d activation d'un noeud atteint par la boucle des voisins de la construction de l'arbre.
    //L'intersection des voisinages n'est à tester que lorsqu'un noeud est établi. Ainsi tous les noeuds dont le pere est actif sont etablis, meme si eux memes sont passifs.
    //printf("\n\nEST ACTIF ?\n");
    if(!dist[pere[noeud]].actif)
        return false;
    return true;
}

bool estActifEt(int noeud)//noeud est % nbNoeuds
{
    if(!dist[noeud].actif)//la date est la bonne car le noeud a ete atteint donc mis a jour
        return false;
    int nbNComm = 0; //nb noeuds a la fois dans h-vois de ancetre et de noeud
    //printf("ancetre : %d\n", ancetre[noeud]);
    for(unsigned int k = 0; k < hVois[ancetre[noeud]].size() - 1; k++)
    {
        //printf("elt :%d", hVois[ancetre[noeud]][k]);
        if(estDedans(hVois[ancetre[noeud]][k], hVois[noeud]))
        {
            nbNComm++;
            if(nbNComm > 1)
                return true;
        }
    }
    return false;
}

vector <int> ajouteArretes(int curN, int source, int dateAppelFct)//idN sont pas modulo NB_NOEUDS
{
    //printf("curN %d source %d\n", curN, source);
    vector <int> feuilles;
    int idNiveau = curN / NB_NOEUDS;//designe le niveau en cours de construction
    int curNMod = curN % NB_NOEUDS; //pour acceder aux tableaux pere et fils

    if(fils[curNMod].date != dateAppelFct || fils[curNMod].enfants.empty())//le noeud est une feuille
    {
        feuilles.push_back(curN);
        return feuilles;
    }

    //printf("fils de la bonne date\n");
    for(unsigned int k = 0; k < fils[curNMod].enfants.size(); k++)
    {
        vector <int> res = ajouteArretes(fils[curNMod].enfants[k] + idNiveau * NB_NOEUDS, source, dateAppelFct);
        for(unsigned int i = 0; i < res.size(); i++)
            feuilles.push_back(res[i]);//on travaille sur un arbre donc pas besoin de verifier si il y a des doublons.
    }
        //printf("OK\n");
    for(unsigned int k = 0; k < feuilles.size(); k++)
    {
        //printf("feuille : %d, curN dans hVois de source : %d, pere de curN dans hVois de feuille %d\n", feuilles[k], estDedans(curNMod, hVois[source % NB_NOEUDS]), dejaAjChem(pere[curNMod] + idNiveau * NB_NOEUDS, curN));
        if(!estDedans(curNMod, hVois[source % NB_NOEUDS])
            && !estDedans(pere[curNMod], hVois[feuilles[k] % NB_NOEUDS])
            && !dejaAjChem(pere[curNMod] + idNiveau * NB_NOEUDS, curN))
        {
            //printf("pere de curN : %d et curN %d et curN % NB_N %d\n", pere[curNMod], curN, curN % NB_NOEUDS);
            chemins[curN].push_back(Way(pere[curNMod] + idNiveau * NB_NOEUDS, longChem(curN - NB_NOEUDS, pere[curNMod] + NB_NOEUDS * (idNiveau - 1))));//car les chemins n'existent pas encore a ce niveau
            chemins[pere[curNMod] + idNiveau * NB_NOEUDS].push_back(Way(curN, longChem(curN - NB_NOEUDS, pere[curNMod] + NB_NOEUDS * (idNiveau - 1))));
            cheminComplet[Route(curN, pere[curNMod] + idNiveau * NB_NOEUDS)] = cheminComplet[Route(curN - NB_NOEUDS, pere[curNMod] + NB_NOEUDS * (idNiveau - 1))];
            cheminComplet[Route(pere[curNMod] + idNiveau * NB_NOEUDS, curN)] = cheminComplet[Route(curN - NB_NOEUDS, pere[curNMod] + NB_NOEUDS * (idNiveau - 1))];
        }
    }
    //printf("feuilles : ");
    //for(unsigned int k = 0; k < feuilles.size(); k++)

    //printf("\n retour au niveau precedent\n");
    return feuilles;
}

void construitNiveau(int idNiveau)
{
    //remplit hVois qui depend du niveau
    for(int source = 0; source < NB_NOEUDS; source++)
    {
        hVois[source] = dHEtHVois(source, idNiveau - 1);
        /*if(idNiveau == 15)
            printf("%d ", source);*/
    }
    //printf("ok");
    /*for(int i = 0; i < 6; i++)
    {
        printf("\n\nhVois de %d: \n", i);
        for(unsigned int k = 0; k < hVois[i].size(); k++)
            printf("%d ", hVois[i][k]);
    }*/
    int correctif = idNiveau * NB_NOEUDS; //val a ajouter a idN pour avoir sa vraie valeur (pas % NB_NOEUDS)
    int correctifInf = (idNiveau - 1) * NB_NOEUDS;//val a ajouter pour atteindre le meme noeud au niveau inferieur et pas de pb car idNiveau >= 1

    for(int source = 0; source < NB_NOEUDS; source++)
    //si le noeud n'appartient pas au niveau, il na pas de voisisns donc pas de probleme : il ne se passe rien
    //tous les id de noeuds sont % NB_NOEUDS
    {
        //printf("\n\n\n\n\nChangement de source : %d\n", source);
        //printf("%d correctif inf\n\n", correctifInf = (idNiveau - 1) * NB_NOEUDS);
        //CONSTRUCTION DE L ARBRE
        if(source % 1000 == 0)
            printf("%d source : %d\n", idNiveau, source);
        int dateAppelFct = donneDate();
        int nbActifs = 1;
        int dateAjNoeud = 0;
        priority_queue <SNoeud> nAV;

        nAV.push(SNoeud(source, 0, dateAjNoeud));
        dateAjNoeud++;
        //pere[source].date = dateAppelFct;
        pere[source] = source;
        dist[source].val = 0;
        dist[source].actif = true;
        dist[source].date = dateAppelFct;
        dist[source].dejaTraite = false;
        fils[source].date = dateAppelFct;
        fils[source].enfants.clear();
        ancetre[source] = source;
        //ancetre[source].date = dateAppel

        while(!nAV.empty() && nbActifs > 0)
        {
            SNoeud curN = nAV.top();
            nAV.pop();
            /*Si le noeud a ete ajoute plusieurs fois dans la file, c'est grave ?
             * oui, on aura des fils en doublon et le compte des noeuds actifs devient faux :
             * comme dans le parcours des voisins, nbActifs est mis a jour si on revient sur un noeud deja vu,
             * si on re enleve un actif on fausse le compte donc il faut un dejaTraite dans le tableau dist
             */
            if (!dist[curN.idN].dejaTraite)
            {

                dist[curN.idN].dejaTraite = true;
                if(dist[curN.idN].actif)
                    nbActifs--;
                //printf("etait actif : %d ", dist[curN.idN].actif);
                dist[curN.idN].actif = estActifEt(curN.idN);
                //printf("curN : %d distADep : %d actif %d nbActifsRestants : %d ancetre %d\n", curN.idN, curN.distADep, dist[curN.idN].actif, nbActifs, ancetre[curN.idN]);
                if(fils[curN.idN].date != dateAppelFct)//si on arrice sur ce noeud pour la premiere fois, on nettoie ses fils, car a cette date on ne s'en est encore jamais servi
                {
                    //printf("    on a nettoye les fils de %d\n", curN.idN);
                    fils[curN.idN].date = dateAppelFct;
                    fils[curN.idN].enfants.clear();
                }
                if(curN.idN != source)
                {
                    fils[pere[curN.idN]].enfants.push_back(curN.idN);
                    //printf("    pere %d a gagne le fils %d\n", pere[curN.idN], curN.idN);
                }

                for(unsigned int k = 0; k < chemins[curN.idN + correctifInf].size(); k++)
                {
                    int nouvDist =  chemins[curN.idN + correctifInf][k].poids + curN.distADep;
                    int idVois = chemins[curN.idN + correctifInf][k].arr % NB_NOEUDS; //car on ne travaille jamais que sur un niveau a la fois
                    //printf("    idVois : %d nouv Dist %d chem : %d\n", idVois, nouvDist, chemins[curN.idN + correctifInf][k].poids);
                    if(dist[idVois].date != dateAppelFct)
                        dist[idVois].dejaTraite = false;
                    if(dist[idVois].date != dateAppelFct || dist[idVois].val > nouvDist)
                    {
                        //printf("        idVois : %d\n", idVois);
                        nAV.push(SNoeud(idVois, nouvDist, dateAjNoeud));
                        dateAjNoeud++;

                        bool etaitActif = (dist[idVois].date == dateAppelFct && dist[idVois].actif);

                        dist[idVois].val = nouvDist;
                        dist[idVois].date = dateAppelFct;
                        pere[idVois] = curN.idN;
                        dist[idVois].actif = dist[curN.idN].actif;//ici le noeud est juste atteint, il prend donc l 'etat d 'activaiton de son pere
                        //pere[idVois].date = dateAppelFct;
                        if(curN.idN == source)
                            ancetre[idVois] = idVois;
                        else
                            ancetre[idVois] = ancetre[curN.idN];

                        if(dist[idVois].actif && !etaitActif)
                            nbActifs++;
                        if(etaitActif && !dist[idVois].actif)
                            nbActifs--;
                            //printf("    id Vois %d etaitA : %d estA : %d nbAct %d\n", idVois, etaitActif, dist[idVois].actif, nbActifs);
                        //printf("Actifs : %d\n", nbActifs);
                    }
                }
            }
        }
        //MISE A JOUR DE E(id niveau)
        /*printf("source %d", source);
        for(int curN = 0; curN < NB_NOEUDS; curN++)//12 car c'est la cas dans le test que j'utilise
            for(unsigned int i = 0; i < fils[curN].enfants.size(); i++)
                printf("    curN : %d fils %d \n", curN, fils[curN].enfants[i]);*/
        /*for(int curN = 0; curN < 12; curN++)
            printf("pere %d de %d\n", pere[curN], curN);*/
        ajouteArretes(source + correctif, source + correctif, dateAppelFct);
    }
}

int main()
{
    litGraphe();

    //TEST DE LA LECTURE DE L ENTREE
    /*for(int k = 0; k < NB_NOEUDS; k++)
        for(unsigned int i = 0; i < chemins[k].size(); i++)
            fprintf(fout, "%d %d\n", k, chemins[k][i].arr);*/

    /*for(int k = 0; k < NB_NOEUDS * 2; k++)
        for(unsigned int i = 0; i < chemins[k].size(); i++)
            printf("%d %d %d\n", k, chemins[k][i].arr, chemins[k][i].poids);*/

    //TEST DE DHETHVOIS

    //construitNiveau(1);

    //printf("%d %d", dejaAjChem(4, 6), dejaAjChem(5, 9));

    time_t tempsDep;
    time(&tempsDep);
    int idC = 0;
    int nbC = 0;
    //printf("lalala");
    compacteChemins(0);
    /*for(int k = NB_NOEUDS; k < NB_NOEUDS; k++)
        for(unsigned int i = 0; i < chemins[k].size(); i++)
        {
            if(k < chemins[k][i].arr)
            {
                fprintf(fout, "noeud :%d et arr %d\n", k, chemins[k][i].arr);
                vector <int> idRs = cheminComplet[Route(k, chemins[k][i].arr)];
                for(int i = 0; i < idRs.size(); i++)
                {
                    fprintf(fout, "%d\n", idRs[i]);
                    //printf("%d ", idRs[i]);
                }
            }
        }*/

    for(int curN = 1; curN < NB_NIVEAUX; curN++)
    {
        construitNiveau(curN);
        compacteChemins(curN);
        //printf("%d ", curN);
        for(int k = curN * NB_NOEUDS; k < (curN + 1) * NB_NOEUDS; k++)
            for(unsigned int i = 0; i < chemins[k].size(); i++)
            {
                if(k < chemins[k][i].arr)
                {
                    nbC++;
                    vector <int> idRs = cheminComplet[Route(k, chemins[k][i].arr)];
                    for(int i = 0; i < idRs.size(); i++)
                    {
                        fprintf(fout, "%d;%d;%d\n", idC, idRs[i], curN);
                        idC++;
                    }
                }
            }
            fprintf(fout2, "niv : %d et nbC : %d\n", curN, nbC);
            nbC = 0;
    }
    /*for(unsigned int curN = NB_NOEUDS; curN < NB_NOEUDS * 2; curN++)
            for(unsigned int k = 0; k < chemins[curN].size(); k++)
                printf("dep : %d, arr : %d; poids : %d\n", curN - NB_NOEUDS, chemins[curN][k].arr - NB_NOEUDS, chemins[curN][k].poids);*/

    /*for(int curN = 0; curN < NB_NIVEAUX; curN++)
    {
        //fprintf(fout, "\n\n%d\n", curN);
        int nbC = 0;
        for(int k = curN * NB_NOEUDS; k < (curN + 1) * NB_NOEUDS; k++)
            for(unsigned int i = 0; i < chemins[k].size(); i++)
            {
                fprintf(fout, "0 %d %d %d\n", chemins[k][i].poids, k - curN * NB_NOEUDS, chemins[k][i].arr - curN * NB_NOEUDS);
                nbC++;
            }
        fprintf(fout, "curN : %d %d\n", curN, nbC);
    }*/

   /*vector <int> res = dHEtHVois(16, 0);
    for(unsigned int k = 0; k < res.size(); k++)
        printf("%d ", res[k]);*/

    time_t tempsFin;
    time(&tempsFin);

    fprintf(fout2, "temps exec : %f", (double) tempsFin -(double) tempsDep);

    return 0;
}




