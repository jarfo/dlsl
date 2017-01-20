#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <float.h>

/* Copio de version anterior para mejorar ayuda -Tony */
char *optasa[] = {
   "-pra",
   "-pre",
   "-pa",
   "-ip",
   "-ie",
   "-psb",
   "-iep",
};

#define OPTASAS 7

char *extasa[] = {
   "Porcentaje real de aciertos",
   "Porcentaje real de errores",
   "Porcentaje de aciertos",
   "Indice de precision",
   "Indice de error",
   "Porcentaje de sustituciones y borrados",
   "Indice de error ponderado"
};

char *formtasa[] = {
   "Ac/(Sust+Ins+Borr+Ac)",
   "(Sust+Ins+Borr)/(Sust+Ins+Borr+Ac)",
   "Ac/(Sust+Borr+Ac)",
   "(Ac-Ins)/(Sust+Borr+Ac)",
   "(Sust+Ins+Borr)/(Sust+Borr+Ac)",
   "(Sust+Borr)/(Sust+Borr+Ac)",
   "(Sust+.5Ins+.5Borr)/(Ac+Sust+Borr)"
};

/* Fin copia */

#define reemplazamiento 1
#define insercion 2
#define borrado 3
#define fin -1
#define OPP "-p"
#define OPAYUDA "-h"
#define OPFICHSIM "-w"
#define OPSEPFRASES "-f"
#define OPSEPSIMBOLOS "-s"
#define OPCONFCOMP "-C"
#define OPCONFNONULA "-c"
#define PRE "-pre"
#define PRA "-pra"
#define PA "-pa"
#define IP "-ip"
#define IE "-ie"
#define PSB "-psb"
#define IEP "-iep"
#define IAP "-iap"
#define FRASESOK "-F"
#define ABSOLUTOS "-A"

typedef struct {
   int  talla;
   int *simbolo;
}    tipo_cadena;



typedef struct {
   int  talla;
   tipo_cadena *cadena;
}    lista_de_cadenas;


typedef struct {
   int  talla;
   lista_de_cadenas *lcadenas;  
}    lista_de_cadenas_alt;


typedef struct forward_nodo_diccionario {
   char *simbolo;
   struct forward_nodo_diccionario *siguiente;
}    nodo_diccionario;

nodo_diccionario *diccionario = NULL;
int  talla_diccionario = 0;
int  diccionario_bloqueado = 0;



int  identificador_simbolo(simbolo)
   char *simbolo;
{
   int  i;
   nodo_diccionario *iterador, *previo;

   if (diccionario == NULL) {
      if (diccionario_bloqueado) {
	 fprintf(stderr,
	     "Error: el simbolo %s no aparece en la declaracion inicial.\n",
		 simbolo);
	 exit(-1);
      }
      diccionario = (nodo_diccionario *) malloc(sizeof(nodo_diccionario));
      diccionario->siguiente = NULL;
      diccionario->simbolo = malloc((strlen(simbolo) + 1) * sizeof(char));
      strcpy(diccionario->simbolo, simbolo);
      talla_diccionario = 1;
      return talla_diccionario;
   } else {
      previo = NULL;
      for (iterador = diccionario, i = 0; iterador;
	    iterador = iterador->siguiente, i++) {
	 if (strcmp(iterador->simbolo, simbolo) == 0)
	    return i + 1;
	 if (strcmp(iterador->simbolo, simbolo) == 0)
	    return i + 1;
	 previo = iterador;
      }
      if (diccionario_bloqueado) {
	 fprintf(stderr,
	     "Error: el simbolo %s no aparece en la declaracion inicial.\n",
		 simbolo);
	 exit(-1);
      }
      iterador = previo->siguiente = (nodo_diccionario *) malloc(sizeof(nodo_diccionario));
      iterador->simbolo = malloc((strlen(simbolo) + 1) * sizeof(char));
      strcpy(iterador->simbolo, simbolo);
      iterador->siguiente = NULL;
      return ++talla_diccionario;
   }
}


char *simbolo_identificador(id)
   int  id;
{
   nodo_diccionario *iterador;
   int  i = 1;

   for (iterador = diccionario; iterador; iterador = iterador->siguiente)
      if (i++ == id)
	 return iterador->simbolo;
   return "!!!";
}

void libera_memoria_diccionario()
{
   nodo_diccionario *iterador, *aux;

   for (iterador = diccionario; iterador; iterador = aux) {
      aux = iterador->siguiente;
      free(iterador->simbolo);
      free(iterador);
   }
}

void bloquea_diccionario()
{
   diccionario_bloqueado = 1;
}

void lee_declaracion_de_simbolos(fichero)
   char *fichero;
{
   FILE *fp;
   char simbolo[1024];
   int  aux;

   fp = fopen(fichero, "r");
   if (fp == NULL) {
      fprintf(stderr, "Fichero %s inexistente\n", fichero);
      exit(-1);
   }
   while (fscanf(fp, "%s", simbolo) > 0)
      aux = identificador_simbolo(simbolo);
   fclose(fp);
   bloquea_diccionario();
}


typedef struct {
   int  dim;
   int **n;
}    matriz_de_confusion;

void malloc_matriz_de_confusion(dim, m)
   int  dim;
   matriz_de_confusion *m;
{
   int  i;

   m->dim = dim;
   m->n = (int **) malloc((dim + 1) * sizeof(int *));
   m->n[0] = (int *) calloc((dim + 1) * (dim + 1), sizeof(int));
   for (i = 1; i <= dim; i++)
      m->n[i] = m->n[i - 1] + dim + 1;
}

void free_matriz_de_confusion(m)
   matriz_de_confusion *m;
{
   m->dim = 0;

   free(m->n[0]);
   free(m->n);
}

void reinicia_matriz_de_confusion(m)
   matriz_de_confusion *m;
{
   int  i, j;
   for (i = 0; i <= m->dim; i++)
      for (j = 0; j <= m->dim; j++)
	 m->n[i][j] = 0;
}

void imprime_matriz_completa(m, fichero)
   matriz_de_confusion *m;
   char *fichero;
{
   nodo_diccionario *iterador;
   int  i, j;
   FILE *fp;

   fp = fopen(fichero, "w");
   fprintf(fp, "\t\t");
   for (iterador = diccionario; iterador != NULL; iterador = iterador->siguiente)
      fprintf(fp, "%s\t", iterador->simbolo);
   fprintf(fp, "\n");
   fprintf(fp, "\t");
   for (j = 0; j <= talla_diccionario; j++)
      fprintf(fp, "%d\t", m->n[0][j]);
   fprintf(fp, "\n");
   for (i = 1, iterador = diccionario; iterador != NULL; i++, iterador = iterador->siguiente) {
      fprintf(fp, "%s\t", iterador->simbolo);
      for (j = 0; j <= talla_diccionario; j++)
	 fprintf(fp, "%d\t", m->n[i][j]);
      fprintf(fp, "\n");
   }
   fclose(fp);
}

void imprime_elementos_no_nulos(m, fichero)
   matriz_de_confusion *m;
   char *fichero;
{
	int  i, j;
	FILE *fp;

	fp = fopen(fichero, "w");
	for (i = 0; i <= talla_diccionario; i++)
		for (j = 0; j <= talla_diccionario; j++) {
			if (m->n[i][j] != 0) {
				if (i == 0)
					fprintf(fp, " -> %s : %d\n", simbolo_identificador(j),
					m->n[i][j]);
				else if (j == 0)
					fprintf(fp, " %s -> : %d\n", simbolo_identificador(i),
					m->n[i][j]);
				else
					fprintf(fp, " %s -> %s : %d\n",
					simbolo_identificador(i), simbolo_identificador(j),
					m->n[i][j]);
			}
		}
		fclose(fp);
}


int **va;
double **d, **dd;


double gp(p, c, s, ns, ni, nb, na, conf)
   double p;
   tipo_cadena c, s;
   int *ns, *ni, *nb, *na;
   matriz_de_confusion *conf;
{
   double dsa, di, db;
   double gs, gi, gb, ga;
   int  i, j;

   gs = 1.0;
   gi = gb = p;
   ga = 0.0;
   *ns = *ni = *nb = *na = 0;
   va[0][0] = fin;
   d[0][0] = 0.0;
   for (i = 1; i <= c.talla; i++) {
      d[i][0] = d[i - 1][0] + gb;
      va[i][0] = borrado;
   }
   for (j = 1; j <= s.talla; j++) {
      d[0][j] = d[0][j - 1] + gi;
      va[0][j] = insercion;
   }
   for (i = 1; i <= c.talla; i++) {
      for (j = 1; j <= s.talla; j++) {
	 dsa = d[i - 1][j - 1] + ((c.simbolo[i] == s.simbolo[j]) ? ga : gs);
	 di = d[i][j - 1] + gi;
	 db = d[i - 1][j] + gb;
	 if (dsa <= di) {
	    if (dsa <= db) {
	       d[i][j] = dsa;
	       va[i][j] = reemplazamiento;
	    } else {
	       d[i][j] = db;
	       va[i][j] = borrado;
	    }
	 } else {
	    if (di < db) {
	       d[i][j] = di;
	       va[i][j] = insercion;
	    } else {
	       d[i][j] = db;
	       va[i][j] = borrado;
	    }
	 }
      }
   }
   i = c.talla;
   j = s.talla;
   do {
      switch (va[i][j]) {
      case reemplazamiento:
	 conf->n[c.simbolo[i]][s.simbolo[j]]++;
	 if (c.simbolo[i] == s.simbolo[j])
	    (*na)++;
	 else
	    (*ns)++;
	 i--;
	 j--;
	 break;
      case insercion:
	 conf->n[0][s.simbolo[j]]++;
	 (*ni)++;
	 j--;
	 break;
      case borrado:
	 conf->n[c.simbolo[i]][0]++;
	 (*nb)++;
	 i--;
	 break;
      }
   } while (va[i][j] != fin);

   return(d[c.talla][s.talla]);
}

void Gp(p, C, S, ns, ni, nb, na, conf, nfc)
   double p;
   lista_de_cadenas_alt C;
   lista_de_cadenas S;
   int *ns, *ni, *nb, *na, *nfc;
   matriz_de_confusion *conf;
{
   int  i, nns, nni, nnb, nna;
   int  j, nnns, nnni, nnnb, nnna;

   double optimo, coste;

   *ns = *ni = *nb = *na = *nfc = 0;
   for (i = 0; i < C.talla; i++) {
      nns = nni = nnb = nna = INT_MAX;
      for (j=0, optimo = FLT_MAX; j< C.lcadenas[i].talla; j++) {
        coste = gp(p, C.lcadenas[i].cadena[j], S.cadena[i], &nnns, &nnni, &nnnb, &nnna, conf);

#ifdef SI_DEBUG
        printf ("Frase: %d, alternativa: %d, ns: %d, ni: %d, nb: %d, na: %d, coste: %f\n",
                 i,j,nnns,nnni,nnnb,nnna,coste);
#endif
        if (optimo > coste) {
           optimo = coste;
           nns = nnns; nni = nnni; nnb = nnnb; nna = nnna;
        }
      }
      *ns += nns;
      *ni += nni;
      *nb += nnb;
      *na += nna;
      if ((nns + nni + nnb) == 0) (*nfc)++; /*utilizar optimo==0 */
#ifdef SI_DEBUG
      printf ("Frase: %d, la mejor:  ns: %d, ni: %d, nb: %d, na: %d, coste: %f\n",
                 i,nns,nni,nnb,nna,optimo);
      printf ("...y por tanto:  ns: %d, ni: %d, nb: %d, na: %d: %f\n",
                 nns,nni,nnb,nna);
#endif
   }
}


double tasa_generica(ns, ni, nb, na, alphas, alphai, alphab, alphaa,
		          betas, betai, betab, betaa)
   int  ns, ni, nb, na;
   double alphas, alphai, alphab, alphaa, betas, betai, betab, betaa;
{

   return 100.0 * (alphas * ns + alphai * ni + alphab * nb + alphaa * na) /
      (betas * ns + betai * ni + betab * nb + betaa * na);
}


double tasa_pra(ns, ni, nb, na)
   int  ns, ni, nb, na;
{
   return tasa_generica(ns, ni, nb, na,
			0.0, 0.0, 0.0, 1.0,
			1.0, 1.0, 1.0, 1.0);
}

double tasa_pre(ns, ni, nb, na)
   int  ns, ni, nb, na;
{
   return tasa_generica(ns, ni, nb, na,
			1.0, 1.0, 1.0, 0.0,
			1.0, 1.0, 1.0, 1.0);
}

double tasa_pa(ns, ni, nb, na)
   int  ns, ni, nb, na;
{
   return tasa_generica(ns, ni, nb, na,
			0.0, 0.0, 0.0, 1.0,
			1.0, 0.0, 1.0, 1.0);
}

double tasa_ip(ns, ni, nb, na)
   int  ns, ni, nb, na;
{
   return tasa_generica(ns, ni, nb, na,
			0.0, -1.0, 0.0, 1.0,
			1.0, 0.0, 1.0, 1.0);
}


double tasa_ie(ns, ni, nb, na)
   int  ns, ni, nb, na;
{
   return tasa_generica(ns, ni, nb, na,
			1.0, 1.0, 1.0, 0.0,
			1.0, 0.0, 1.0, 1.0);
}


double tasa_psb(ns, ni, nb, na)
   int  ns, ni, nb, na;
{
   return tasa_generica(ns, ni, nb, na,
			1.0, 0.0, 1.0, 0.0,
			1.0, 0.0, 1.0, 1.0);
}

double tasa_iep(ns, ni, nb, na)
   int  ns, ni, nb, na;
{
   return tasa_generica(ns, ni, nb, na,
			1.0, 0.5, 0.5, 0.0,
			1.0, 0.0, 1.0, 1.0);
}


double tasa_iap(ns, ni, nb, na)
   int  ns, ni, nb, na;
{
   return tasa_generica(ns, ni, nb, na,
			0.0, -0.5, 0.5, 1.0,
			1.0, 0.0, 1.0, 1.0);
}




double sub_inicializa_lambda(c, s, ns, ni, nb, na)
   tipo_cadena c, s;
   int *ns, *ni, *nb, *na;
{
   double dsa, di, db;
   double ddsa, ddi, ddb;
   double fsa, fi, fb;
   double gs, gi, gb, ga;
   double coste;
   int  i, j;

   gs = 1.0;
   gi = gb = 1.0;
   ga = 0.0;

   *ns = *ni = *nb = *na = 0;
   va[0][0] = fin;
   d[0][0] = 0.0;
   dd[0][0] = 0.0;
   for (i = 1; i <= c.talla; i++) {
      d[i][0] = d[i - 1][0] + gb;
      dd[i][0] = dd[i - 1][0] + 1;
      va[i][0] = borrado;
   }
   for (j = 1; j <= s.talla; j++) {
      d[0][j] = d[0][j - 1] + gi;
      dd[0][j] = dd[0][j - 1] + 1;
      va[0][j] = insercion;
   }
   for (i = 1; i <= c.talla; i++) {
      for (j = 1; j <= s.talla; j++) {
	 dsa = d[i - 1][j - 1] + ((c.simbolo[i] == s.simbolo[j]) ? ga : gs);
	 di = d[i][j - 1] + gi;
	 db = d[i - 1][j] + gb;
	 ddsa = dd[i - 1][j - 1] + 1;
	 ddi = dd[i][j - 1] + 1;
	 ddb = dd[i - 1][j] + 1;
	 fsa = dsa / ddsa;
	 fi = di / ddi;
	 fb = db / ddb;
	 if (fsa <= fi) {
	    if (fsa <= fb) {
	       d[i][j] = dsa;
	       dd[i][j] = ddsa;
	       va[i][j] = reemplazamiento;
	    } else {
	       d[i][j] = db;
	       dd[i][j] = ddb;
	       va[i][j] = borrado;
	    }
	 } else {
	    if (fi < fb) {
	       d[i][j] = di;
	       dd[i][j] = ddi;
	       va[i][j] = insercion;
	    } else {
	       d[i][j] = db;
	       dd[i][j] = ddb;
	       va[i][j] = borrado;
	    }
	 }
      }
   }

   i = c.talla;
   j = s.talla;
   coste = d[i][j]/dd[i][j];
   do {
      switch (va[i][j]) {
      case reemplazamiento:
	 if (c.simbolo[i] == s.simbolo[j])
	    (*na)++;
	 else
	    (*ns)++;
	 i--;
	 j--;
	 break;
      case insercion:
	 (*ni)++;
	 j--;
	 break;
      case borrado:
	 (*nb)++;
	 i--;
	 break;
      }
   } while (va[i][j] != fin);
   return (coste);
}

double inicializa_lambda(C, S)
   lista_de_cadenas_alt C;
   lista_de_cadenas S;
{
   int  n, ns, ni, nb, na;
   int  nns, nni, nnb, nna;
   int  j, nnns, nnni, nnnb, nnna;

   double coste, optimo;

   ns = ni = nb = na = 0;

   for (n = 0; n < C.talla; n++) {
      nns = nni = nnb = nna = INT_MAX;

      for (j=0, optimo = FLT_MAX; j< C.lcadenas[n].talla; j++) {
         coste = sub_inicializa_lambda 
                          (C.lcadenas[n].cadena[j], S.cadena[n], &nnns, &nnni, &nnnb, &nnna);
         if (optimo > coste) {
            optimo = coste;
            nns = nnns; nni = nnni; nnb = nnnb; nna = nnna;
         }
      }
      ns += nns;
      ni += nni;
      nb += nnb;
      na += nna;
   }
   return (ns + ni + nb) / (double) (ns + ni + nb + na);
}

void Fp(C, S, ns, ni, nb, na, conf, nfc)
   lista_de_cadenas_alt C;
   lista_de_cadenas S;
   int *ns, *ni, *nb, *na, *nfc;
   matriz_de_confusion *conf;
{
   double lambda, lambdacero;
   double p;

   lambda = inicializa_lambda(C, S);
   do {
      lambdacero = lambda;
      p = 1.0 - lambdacero / 2.0;
      reinicia_matriz_de_confusion(conf);
      Gp(p, C, S, ns, ni, nb, na, conf, nfc);
      lambda = (*ns + *ni + *nb) / (double) (*ni + *nb + *ns + *na);
/*      printf("Lambda = %f => p = %f\n",lambda,p);  */
/*      printf("Lambda = %f => fabs((lambda - lambdacero)/lambda) = %f\n",lambda, fabs((lambda - lambdacero)/lambda)); */
/*   } while (lambda != lambdacero); Adrián */
   } while (fabs((lambda - lambdacero)/lambda) > 1e-6);
}


void ayuda_y_aborta(argc, argv)
   int  argc;
   char *argv[];
{
   fprintf(stderr, "Evaluador de sistema RAH\n");
   fprintf(stderr, "Uso: %s fich [%s \"c\"] [%s #|%s \"c\"]",
	   argv[0], OPSEPFRASES, OPSEPSIMBOLOS, OPSEPSIMBOLOS);
   fprintf(stderr, " [%s #] [TASA] [%s mat|%s mat] [%s dicc]\n",
	   OPP, OPCONFNONULA, OPCONFCOMP, OPFICHSIM);
   fprintf(stderr, "donde: \n");
   fprintf(stderr, "  %s \"c\" : hace que c separe frases en una linea\n", OPSEPFRASES);
   fprintf(stderr, "  %s #   : hace que cada # caracteres se consideren un simbolo\n",
	   OPSEPSIMBOLOS);
   fprintf(stderr, "  %s \"cad\" : hace que \"cad\" separe simbolos\n", OPSEPSIMBOLOS);
   fprintf(stderr, "  %s #   : fija el parametro p a #\n", OPP);
   fprintf(stderr, "  %s     : da ademas el numero de frases correctas\n", FRASESOK);
   fprintf(stderr, "  %s     : da el numero absoluto de ins., borr., etc\n", ABSOLUTOS);
   fprintf(stderr, "  TASA   : proporciona una tasa concreta\n");
   fprintf(stderr, "           [%s|%s|%s|%s|%s|%s|%s|%s] \n\n",
	   PRA, PRE, PA, IP, IE, PSB, IEP, IAP);

   fprintf(stderr, "\t   %s: %s\n\t\t%s\n", PRA, "Porcentaje real de aciertos",
	   "Ac/(Sust+Ins+Borr+Ac)");
   fprintf(stderr, "\t   %s: %s\n\t\t%s\n", PRE, "Porcentaje real de errores",
	   "(Sust+Ins+Borr)/(Sust+Ins+Borr+Ac)");
   fprintf(stderr, "\t   %s: %s\n\t\t%s\n", PA, "Porcentaje de aciertos",
	   "Ac/(Sust+Borr+Ac)");

   fprintf(stderr, "\t   %s: %s\n\t\t%s\n", IP, "Indice de precision",
	   "(Ac-Ins)/(Sust+Borr+Ac)");

   fprintf(stderr, "\t   %s: %s\n\t\t%s\n", IE, "Indice de error",
	   "(Sust+Ins+Borr)/(Sust+Borr+Ac)");

   fprintf(stderr, "\t   %s: %s\n\t\t%s\n", PSB, "Porcentaje de sustituciones y borrados", "(Sust+Borr)/(Sust+Borr+Ac)");


   fprintf(stderr, "\t   %s: %s\n\t\t%s\n", IEP, "Indice de error ponderado",
	   "(Sust+.5Ins+.5Borr)/(Ac+Sust+Borr)");

   fprintf(stderr, "\t   %s: %s\n\t\t%s\n\n", IAP, " Indice de acierto ponderado", " (Ac+.5Borr-.5Ins)/(Ac+Sust+Borr) ");

   fprintf(stderr, "  %s mat : guarda en fichero mat la matriz de confusion\n", OPCONFCOMP);
   fprintf(stderr, "  %s mat : guarda en mat elementos no nulos de matriz de confusion\n",
	   OPCONFNONULA);
   fprintf(stderr, "  %s dicc: toma del fichero dicc el orden de los simbolos \n",
	   OPFICHSIM);
   fprintf(stderr, "POR DEFECTO: %s %s \"*\" %s 1 %s\n",
	   argv[0], OPSEPFRASES, OPSEPSIMBOLOS, PRA);

   exit(-1);
}

void cadena_sin_separadores(linea, desde, hasta, talla_simbolo, c)
   char *linea;
   int  desde, hasta, talla_simbolo;
   tipo_cadena *c;
{
   int  j, k;
   char simbolo[1024];

   c->talla = (hasta - desde) / talla_simbolo;
   c->simbolo = (int *) malloc((1 + c->talla) * sizeof(int));
   for (j = 0; j < c->talla; j++) {
      for (k = 0; k < talla_simbolo; k++)
	 simbolo[k] = linea[j * talla_simbolo + k + desde];
      simbolo[k] = '\0';
      c->simbolo[j + 1] = identificador_simbolo(simbolo);
   }
}

int  es_separador(caracter, separadores)
   char caracter, *separadores;
{
   int  i, j;
   j = strlen(separadores);
   for (i = 0; i < j; i++)
      if (caracter == separadores[i])
	 return 1;
   return 0;
}

void cadena_con_separadores(linea, desde, hasta, separadores, c)
   char *linea;
   int  desde, hasta;
   char *separadores;
   tipo_cadena *c;
{
   int  i, j, k;
   char simbolo[1024];

   c->talla = 0;
   for (j = desde; es_separador(linea[j], separadores); j++);
   while (j < hasta) {
      while ((j < hasta) && !es_separador(linea[j], separadores))
	 j++;
      while ((j < hasta) && es_separador(linea[j], separadores))
	 j++;
      c->talla++;
   }
   if (c->talla > 0) {
      c->simbolo = (int *) malloc((1 + c->talla) * sizeof(int));
	  c->simbolo[0] = 0;
      i = 1;
      for (j = desde; es_separador(linea[j], separadores); j++);
      while (j < hasta) {
	 k = 0;
	 while ((j < hasta) && !es_separador(linea[j], separadores))
	    simbolo[k++] = linea[j++];
	 simbolo[k] = '\0';
	 c->simbolo[i++] = identificador_simbolo(simbolo);
	 while ((j < hasta) && es_separador(linea[j], separadores))
	    j++;
      }
   }
   else {
	 c->simbolo = NULL;
   }
}

void ver_datos(C,S) 
   lista_de_cadenas     S;
   lista_de_cadenas_alt C;
{
   int frase,alternativa,simb;

   for (frase=0; frase< C.talla;frase++) {
       for (alternativa = 0; alternativa < C.lcadenas[frase].talla; alternativa++) {
           for (simb = 0; simb <= C.lcadenas[frase].cadena[alternativa].talla;
                simb++)
                printf("%d ", C.lcadenas[frase].cadena[alternativa].simbolo[simb]);
           printf("@ ");
       }
       printf("$");
       for (simb = 0; S.cadena[frase].simbolo != NULL && simb <= S.cadena[frase].talla;
            simb++)
            printf("%d ", S.cadena[frase].simbolo[simb]);
       printf("\n");
    }

}


void lee_datos(fichero, C, S,
	         separador_de_cadenas, talla_simbolo, separador_de_simbolos)
   char *fichero;
   lista_de_cadenas     *S;
   lista_de_cadenas_alt *C;
   char separador_de_cadenas;
   int  talla_simbolo;
   char *separador_de_simbolos;
{
   FILE *fp;
   char linea[2048];
   int  j, talla_linea, fin_c = 0, inicio_c, inicio_s;
   int  nsep, nalt;


   fp = fopen(fichero, "r");
   if (fp == NULL) {
      fprintf(stderr, "Fichero %s inexistente\n", fichero);
      exit(-1);
   }
   j = 0;
   while (fscanf(fp, "%s", linea) > 0)
      j++;
   fclose(fp);

   /* ??????????????????? C->cadena = (tipo_cadenas *) malloc(j * sizeof(int
    * *)); S->cadena = (tipo_cadena *) malloc(j * sizeof(int *)); */


   C->lcadenas = (lista_de_cadenas *) malloc(j * sizeof(lista_de_cadenas));
   S->cadena = (tipo_cadena *) malloc(j * sizeof(tipo_cadena));


   fp = fopen(fichero, "r");
   C->talla = S->talla = 0;
   while (fgets(linea, 2048, fp) != NULL) {
      talla_linea = strlen(linea);
      if (linea[talla_linea - 1] == '\n') 
	 linea[--talla_linea] = '\0';
      for (nsep = j = 0; j < talla_linea; j++)
	 if (linea[j] == separador_de_cadenas)
	    nsep++;
      if (nsep == 0) {
	 fprintf(stderr, "No hay separador de cadenas (%c) en la linea %d.\n",
		 separador_de_cadenas, C->talla + 1);
	 fprintf(stderr, "\"%s\"\n", linea);
	 exit(-1);
      }

      C->lcadenas[C->talla].talla = nsep;
      C->lcadenas[C->talla].cadena = (tipo_cadena *) malloc(nsep * sizeof(tipo_cadena));

      for (nalt = inicio_c = j = 0; j < talla_linea && nalt < nsep; j++) {
	 if (linea[j] == separador_de_cadenas) {
            fin_c = j;
            if (talla_simbolo == 0)
	       cadena_con_separadores(linea, inicio_c, fin_c, separador_de_simbolos,
				&(C->lcadenas[C->talla].cadena[nalt]));
            else 
	       cadena_sin_separadores(linea, 0, fin_c, talla_simbolo,
				&C->lcadenas[C->talla].cadena[nalt]);
            inicio_c = fin_c + 1;
            nalt++;
         }
      }

      inicio_s = j + 1;
      if (talla_simbolo == 0) 
	 cadena_con_separadores(linea, inicio_s, talla_linea, separador_de_simbolos,
				&S->cadena[S->talla]);
      else 
	 cadena_sin_separadores(linea, 0, fin_c, talla_simbolo,
				&S->cadena[S->talla]);
      C->talla++;
      S->talla++;
   }
   fclose(fp);
}




int main(argc, argv)
   int  argc;
   char *argv[];
{
   lista_de_cadenas S;
   lista_de_cadenas_alt C;

   matriz_de_confusion conf;
   int  i, j, max_s, max_c, talla_simbolo, ns, ni, nb, na, nfc;
   char fichero[256], fichero_simbolos[256], fichero_matriz[256], con_matriz, *tasa,
        normalizado=0, separador_de_cadenas, separador_de_simbolos[256],frases_correctas=0, absolutos=0;
   double p, rtasa = 0;
   float tot;



   separador_de_cadenas = '*';
   talla_simbolo = 1;
   strcpy(fichero, "");
   strcpy(fichero_simbolos, "");
   strcpy(fichero_matriz, "");
   con_matriz = 0;
   tasa = PRE;
   p = -1e30;
   if (argc == 1)
      ayuda_y_aborta(argc, argv);
   else {
      for (i = 1; i < argc; i++) {
	 if (strcmp(argv[i], OPAYUDA) == 0) {
	    ayuda_y_aborta(argc, argv);
	 }
	 if (strcmp(argv[i], OPP) == 0 && normalizado != 2) {
	    normalizado = 0;
	    p = atof(argv[i + 1]);
	    i++;
	    continue;
	 }
	 if (strcmp(argv[i], OPFICHSIM) == 0) {
	    strcpy(fichero_simbolos, argv[++i]);
	    continue;
	 }
	 if (strcmp(argv[i], OPSEPFRASES) == 0) {
	    separador_de_cadenas = argv[++i][0];
	    continue;
	 }
	 if (strcmp(argv[i], FRASESOK) == 0) {
	    frases_correctas++;
	    continue;
	 }
	 if (strcmp(argv[i], ABSOLUTOS) == 0) {
	    absolutos++;
	    continue;
	 }
	 if (strcmp(argv[i], OPSEPSIMBOLOS) == 0) {
	    if (atoi(argv[i + 1]) <= 0) {
	       talla_simbolo = 0;
	       strcpy(separador_de_simbolos, argv[++i]);
	    } else {
	       talla_simbolo = atoi(argv[++i]);
	    }
	    continue;
	 }
	 if (strcmp(argv[i], PRA) == 0 || strcmp(argv[i], PRE) == 0 ||
	       strcmp(argv[i], PA) == 0 ||
	       strcmp(argv[i], IP) == 0 || strcmp(argv[i], IE) == 0 ||
	       strcmp(argv[i], PSB) == 0 ||
	       strcmp(argv[i], IEP) == 0 || strcmp(argv[i], IAP) == 0) {
	    tasa = argv[i];
	    continue;
	 }
	 if (strcmp(argv[i], OPCONFNONULA) == 0) {
	    con_matriz = 1;
	    strcpy(fichero_matriz, argv[++i]);
	    continue;
	 }
	 if (strcmp(argv[i], OPCONFCOMP) == 0) {
	    con_matriz = 2;
	    strcpy(fichero_matriz, argv[++i]);
	    continue;
	 }
	 if (strcmp(fichero, "") == 0)
	    strcpy(fichero, argv[i]);
	 else
	    ayuda_y_aborta(argc, argv);
      }
      if (strcmp(fichero, "") == 0)
	 ayuda_y_aborta(argc, argv);
      if (p == -1e30) {
	 if (strcmp(tasa, PRA) == 0 || strcmp(tasa, PRE) == 0)
	    normalizado = 1;
	 else if (strcmp(tasa, PA) || strcmp(tasa, PSB) == 0 || strcmp(tasa, IEP) == 0
		  || strcmp(tasa, IAP) == 0)
	    p = 0.5;
	 else
	    p = 0.5;
      }
   }


   if (strcmp(fichero_simbolos, "") != 0)
      lee_declaracion_de_simbolos(fichero_simbolos);
   lee_datos(fichero, &C, &S, separador_de_cadenas, talla_simbolo, separador_de_simbolos);
/*   ver_datos(C,S);  */


   max_s = 0;
   max_c = 0;
   for (i = 0; i < S.talla; i++)
      max_s = (max_s > S.cadena[i].talla) ? max_s : S.cadena[i].talla;
   for (i = 0; i < C.talla; i++)
      for (j = 0; j < C.lcadenas[i].talla; j++)
         max_c = (max_c > C.lcadenas[i].cadena[j].talla) ? max_c : C.lcadenas[i].cadena[j].talla;


   d = (double **) malloc((max_c + 1) * sizeof(double *));
   d[0] = (double *) malloc((max_c + 1) * (max_s + 1) * sizeof(double));
   for (i = 1; i <= max_c; i++)
      d[i] = d[i - 1] + max_s + 1;
   dd = (double **) malloc((max_c + 1) * sizeof(double *));
   dd[0] = (double *) malloc((max_c + 1) * (max_s + 1) * sizeof(double));
   for (i = 1; i <= max_c; i++)
      dd[i] = dd[i - 1] + max_s + 1;
   va = (int **) malloc((max_c + 1) * sizeof(int *));
   va[0] = (int *) malloc((max_c + 1) * (max_s + 1) * sizeof(int));
   for (i = 1; i <= max_c; i++)
      va[i] = va[i - 1] + max_s + 1;


   malloc_matriz_de_confusion(talla_diccionario, &conf);

   if (normalizado) {
      Fp(C, S, &ns, &ni, &nb, &na, &conf, &nfc);
      if (strcmp(tasa, PRA) == 0)
	 rtasa = tasa_pra(ns, ni, nb, na);
      else if (strcmp(tasa, PRE) == 0)
	 rtasa = tasa_pre(ns, ni, nb, na);
   } else {

      Gp(p, C, S, &ns, &ni, &nb, &na, &conf, &nfc);
      if (strcmp(tasa, PRE) == 0)
	 rtasa = tasa_pre(ns, ni, nb, na);
      else if (strcmp(tasa, PRA) == 0)
	 rtasa = tasa_pra(ns, ni, nb, na);
      else if (strcmp(tasa, PA) == 0)
	 rtasa = tasa_pa(ns, ni, nb, na);
      else if (strcmp(tasa, IP) == 0)
	 rtasa = tasa_ip(ns, ni, nb, na);
      else if (strcmp(tasa, IE) == 0)
	 rtasa = tasa_ie(ns, ni, nb, na);
      else if (strcmp(tasa, PSB) == 0)
	 rtasa = tasa_psb(ns, ni, nb, na);
      else if (strcmp(tasa, IEP) == 0)
	 rtasa = tasa_iep(ns, ni, nb, na);
      else if (strcmp(tasa, IAP) == 0)
	 rtasa = tasa_iap(ns, ni, nb, na);
   }

   tot = (float) (ns+ni+nb+na)/100.0;
   printf("Tasa %s: %f\n", tasa, rtasa);
   printf("Goles: %.2f%%  Subs: %.2f%%  Ins: %.2f%%  Borr: %.2f%%\tTotales:%d\n",
           na/tot,ns/tot,ni/tot,nb/tot,(int) (100*tot));
   if (absolutos)    
      printf("ABS: Goles: %d  Subs: %d  Ins: %d  Borr: %d\n",na,ns,ni,nb);
   if (frases_correctas) 
      printf("Frases correctas: %d/%d => %.2f\n",nfc,C.talla,100.0*nfc/(float) C.talla);
   if (con_matriz == 1)
      imprime_elementos_no_nulos(&conf, fichero_matriz);
   if (con_matriz == 2)
      imprime_matriz_completa(&conf, fichero_matriz);


   free_matriz_de_confusion(&conf);

   free(d[0]);
   free(d);
   free(dd[0]);
   free(dd);
   free(va[0]);
   free(va);


   libera_memoria_diccionario();
   return 0;
}


