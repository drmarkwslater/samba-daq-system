#include <stdlib.h>

#ifndef WIN32
#ifndef __MWERKS__
#define STD_UNIX
#endif
#endif

#ifdef STD_UNIX
#include <strings.h>  /* pour strlen qui rend size_t si intrinseque */
#else
#include <string.h>
#endif

#include <opium.h>
#include <timeruser.h>
#include <html.h>

#ifdef WIN32
extern WndIdent WndRoot;
#endif

#ifdef CW5
/* ========================================================================== */
static int strncasecmp(char *t1, char *t2, int n) {
	char *p1,*p2,c1,c2;
	int k;

	k = n; p1 = t1; p2 = t2;
	while(k--) {
		c1 = *p1++; c2 = *p2++;
		if(!c1 || !c2) break;
		c1 = toupper(c1); c2 = toupper(c2);
		if(c1 != c2) break;
	}
	if(c1 > c2) return(1);
	else if(c1 < c2) return(-1);
	else return(0);
}
#endif

#ifdef WIN32
/* ========================================================================== */
static int strncasecmp(char *t1, char *t2, int n) {
	char *p1,*p2,c1,c2;
	int k;

	k = n; p1 = t1; p2 = t2;
	while(k--) {
		c1 = *p1++; c2 = *p2++;
		if(!c1 || !c2) break;
		c1 = toupper(c1); c2 = toupper(c2);
		if(c1 != c2) break;
	}
	if(c1 > c2) return(1);
	else if(c1 < c2) return(-1);
	else return(0);
}
#endif
/* ========================================================================== */
static Menu MenuCreation(MenuItem *items, char traduit) {
	Cadre cdr; MenuItem *item; Menu menu;
	
	menu = (Menu)malloc(sizeof(TypeMenu));
	if(menu) {
		if(DEBUG_MENU(1)) WndPrint("Menu cree @%08X\n",(hexa)menu);
		cdr = OpiumCreate(OPIUM_MENU,menu);
		if(!cdr) {
			free(menu); return(0); 
		}
		menu->cdr = cdr;
		menu->items = items;
		menu->lig = 0;
		menu->col = 0;
		menu->taille_calculee = 0;
		menu->nbcols = 0;
		//		menu->loc[0] = 0;
		//		menu->larg[0] = 0;
		menu->defaut = 0;
		menu->item_pointe = 0;
		menu->curs = 0;
		menu->nbitems = 0;
		if((item = items)) while(item->texte) {
			if(traduit) item->traduit = L_(item->texte);
			else item->traduit = item->texte;
			menu->nbitems += 1;
			item++;
		}
		menu->maxitems = menu->nbitems;
		menu->hilite = menu->reverse = 0;
		OpiumSetOptions(cdr);
		if(DEBUG_MENU(3)) WndPrint("Utilise cdr @%08X\n",(hexa)menu->cdr);
	}
	
	return(menu);
}
/* ========================================================================== */
Menu MenuCreate(MenuItem *items) {
	return(MenuCreation(items,0));
}
/* ========================================================================== */
Menu MenuLocalise(MenuItem *items) {
	return(MenuCreation(items,1));
}
/* ========================================================================== */
void MenuRelocalise(Menu m) {
	int i; MenuItem *item;

	if(!m) return;
	item = m->items;
	for(i=0; i<m->maxitems; i++, item++) item->traduit = L_(item->texte);
	if(m->cdr) {
		char titre[OPIUM_MAXTITRE];
		strncpy(titre,(m->cdr)->titre,OPIUM_MAXTITRE);
		strncpy((m->cdr)->titre,L_(titre),OPIUM_MAXTITRE);
		(m->cdr)->titre[OPIUM_MAXTITRE - 1] = '\0';
	}
}
/* ========================================================================== */
void MenuTitle(Menu m, char *texte) { OpiumTitle(m->cdr,texte); }
/* ========================================================================== */
Menu MenuIntitule(MenuItem *items, char *texte) {
	Menu m;

#ifdef MENU_BARRE_WIN32
	SetWindowText(WndRoot, texte);
#endif
	if((m = MenuCreation(items,1))) OpiumTitle(m->cdr,L_(texte));

	return(m);
}
/* ========================================================================== */
Menu MenuTitled(char *texte, MenuItem *items) {
	Menu m;
	
#ifdef MENU_BARRE_WIN32
	SetWindowText(WndRoot, texte);
#endif
	if((m = MenuCreate(items))) OpiumTitle(m->cdr,texte);
	
	return(m);
}
/* ========================================================================== */
void MenuColumns(Menu m, int cols) {
//	int i;

	if(m) {
		if(cols > MNU_MAXCOLS) cols = MNU_MAXCOLS;
		m->nbcols = cols;
//		for(i=0; i<cols; i++) { m->loc[i] = 0; m->larg[i] = 0; }
	}
}
/* ========================================================================== */
int MenuItemNum(Menu m, MenuItem *item_cherche) {
	int i; MenuItem *item;

	item = m->items;
	for(i=0; i<m->maxitems; i++, item++) if(item_cherche == item) break;
	if(i == m->maxitems) return(0); else return(i + 1);
}
/* ========================================================================== */
int MenuItemNb(Menu m) { return(m? m->nbitems: 0); }
/* ========================================================================== */
int MenuLargeurPix(Menu m) {
	OpiumSizeMenu(m->cdr,0);
	return((m->cdr)->larg);
}
/* ========================================================================== */
int MenuItemGetIndex(Menu m, IntAdrs num) {
	int i; MenuItem *item;

	if(!m) return(-1);
	if(!num) return(-1);
	if((num < 1) || (num > m->maxitems)) {
		item = m->items;
		for(i=0; i<m->maxitems; i++, item++) if(!strcmp(item->texte,(char *)num)) break;
		if(i == m->maxitems) return(-1);
	} else i = (int)(num - 1);
	return(i);
}
/* ========================================================================== */
char *MenuItemGetText(Menu m, IntAdrs num) {
	int i;

	if((i = MenuItemGetIndex(m,num)) >= 0) return(m->items[i].texte);
	else return("");
}
/* ========================================================================== */
void MenuItemSetText(Menu m, IntAdrs num, char *nouveau) {
	int i;

	if((i = MenuItemGetIndex(m,num)) >= 0) {
		m->items[i].texte = nouveau;
		m->items[i].traduit = m->items[i].texte;
	}
}
/* ========================================================================== */
char *MenuItemGetLocalise(Menu m, IntAdrs num) {
	int i;
	
	if((i = MenuItemGetIndex(m,num)) >= 0) return(m->items[i].traduit);
	else return("");
}
/* ========================================================================== */
void MenuItemSetLocalise(Menu m, IntAdrs num, char *nouveau) {
	int i;
	
	if((i = MenuItemGetIndex(m,num)) >= 0) {
		m->items[i].texte = nouveau;
		m->items[i].traduit = L_(m->items[i].texte);
	}
}
/* ========================================================================== */
WndContextPtr MenuItemGetContext(Menu m, IntAdrs num) {
	int i;

	if((i = MenuItemGetIndex(m,num)) >= 0) return(m->items[i].gc);
	else return(0);
}
/* ========================================================================== */
void MenuItemSetContext(Menu m, IntAdrs num, WndContextPtr gc) {
	int i;

	if((i = MenuItemGetIndex(m,num)) >= 0) m->items[i].gc = gc;
}
/* ========================================================================== */
void MenuItemDefault(Menu m, IntAdrs num) {
	int i;

	if((i = MenuItemGetIndex(m,num)) >= 0) m->defaut = i + 1;
}
/* ========================================================================== */
void MenuItemEnable(Menu m, IntAdrs num) {
	int i;

	if((i = MenuItemGetIndex(m,num)) >= 0) m->items[i].sel = MNU_ITEM_OK;
}
/* ========================================================================== */
void MenuItemDisable(Menu m, IntAdrs num) {
	int i;

	if((i = MenuItemGetIndex(m,num)) >= 0) m->items[i].sel = MNU_ITEM_HS;
}
/* ========================================================================== */
void MenuItemHide(Menu m, IntAdrs num) {
	int i;
	
	if((i = MenuItemGetIndex(m,num)) >= 0) m->items[i].sel = MNU_ITEM_CACHE;
}
/* ========================================================================== */
void MenuItemSelect(Menu m, IntAdrs num, char *nouveau, char sel) {
	int i; char doit_terminer;
	
	if((i = MenuItemGetIndex(m,num)) >= 0) {
		if(nouveau) MenuItemSetText(m,num,nouveau);
		m->items[i].sel = sel;
		if(OpiumAlEcran(m)) {
			doit_terminer = WndRefreshBegin((m->cdr)->f);
			OpiumRefresh(m->cdr);
			if(doit_terminer) WndRefreshEnd((m->cdr)->f);
		}
	}
}
/* ========================================================================== */
void MenuItemAllume(Menu menu, IntAdrs num, char *nouveau, 
	WndColorLevel r, WndColorLevel g, WndColorLevel b) {
	Cadre cdr; WndContextPtr gc; char doit_terminer;

	if(!menu) return;
	cdr = menu->cdr;
	gc = MenuItemGetContext(menu,num);
	// PRINT_GC(gc);
	if(!gc) {
		OpiumDrawingRGB(cdr,&gc,0,0,0);
		MenuItemSetContext(menu,num,gc);
	}
	if(gc && cdr) WndContextBgndRGB(cdr->f,gc,r,g,b);
	// PRINT_GC(gc); PRINT_COLOR(gc->background); PRINT_COLOR(gc->foreground);
	if(nouveau) MenuItemSetText(menu,num,nouveau);
	if(OpiumDisplayed(cdr)) {
		doit_terminer = WndRefreshBegin(cdr->f);
		OpiumRefresh(cdr);
		if(doit_terminer) WndRefreshEnd(cdr->f);
	}
}
/* ========================================================================== */
void MenuItemEteint(Menu menu, IntAdrs num, char *nouveau) {
	Cadre cdr; WndContextPtr gc; char doit_terminer;
	
	if(!menu) return;
	cdr = menu->cdr;
	if((gc = MenuItemGetContext(menu,num))) {
		if(cdr) WndContextFree(cdr->f,gc);
		MenuItemSetContext(menu,num,0);
	}
	if(nouveau) MenuItemSetText(menu,num,nouveau);
	if(OpiumDisplayed(cdr)) {
		doit_terminer = WndRefreshBegin(cdr->f);
		OpiumRefresh(cdr);
		if(doit_terminer) WndRefreshEnd(cdr->f);
	}
}
/* ========================================================================== */
char MenuItemArray(Menu menu, int max) {
	MenuItem *items;

	if(!menu || (max == 0)) return(0);
	items = (MenuItem *)malloc((max + 1) * sizeof(MenuItem));
	if(!items) return(0);
	menu->items = items;
	menu->maxitems = max;
	items->texte = 0;
	return(1);
}
/* ========================================================================== */
int MenuItemAdd(Menu menu, char *texte, char *traduit,
	WndContextPtr gc, char sel, char aff, char key, char type, void *adrs) {
	MenuItem *item;

	if(!menu) return(0);
	if(!(menu->items)) return(0);
	if(menu->nbitems >= menu->maxitems) return(0);
	item = menu->items + menu->nbitems;
	item->texte = texte;
	item->traduit = item->texte;
	item->gc = gc;
	item->sel = sel;
	item->aff = aff;
	item->key = key;
	item->type = type;
	item->adrs = adrs;
	item++; item->texte = 0;
	menu->nbitems += 1;
	return(menu->nbitems);
}
/* ========================================================================== */
int MenuItemReplace(Menu menu, char *texte_origine, char *texte_nouveau, 
				WndContextPtr gc, char sel, char key, char type, void *adrs) {
	MenuItem *item; int i;
	
	if(!menu) return(0);
	if((i = MenuItemGetIndex(menu,(IntAdrs)texte_origine)) < 0) return(0);// menu->maxitems??
	item = menu->items + i;
	item->texte = texte_nouveau;
	item->traduit = item->texte;
	item->gc = gc;
	item->sel = sel;
	item->key = key;
	item->type = type;
	item->adrs = adrs;
	return(i+1); // menu->nbitems??
}
/* ========================================================================== */
int MenuItemCopy(Menu menu, MenuItem *included) {
	MenuItem *item;

	if(!menu) return(0);
	if(!(menu->items)) return(0);
	if(menu->nbitems >= menu->maxitems) return(0);
	item = menu->items + menu->nbitems;
	item->texte = included->texte;
	item->traduit = included->traduit;
	item->gc = included->gc;
	item->sel = included->sel;
	item->key = included->key;
	item->type = included->type;
	item->adrs = included->adrs;
	item++; item->texte = 0;
	menu->nbitems += 1;
	return(menu->nbitems);
}
/* ========================================================================== */
void MenuSupport(Menu menu, char support) {
	if(menu) { if(menu->cdr) (menu->cdr)->support = support; }
}
/* ========================================================================== */
Menu MenuBouton(char *texte, char *traduit, WndContextPtr gc, char sel, char aff, char key, char type, void *adrs) {
	Menu menu;
	
	menu = MenuCreate(0);
	if(MenuItemArray(menu,1)) {
		MenuItemAdd(menu,texte,traduit,gc,sel,aff,key,type,adrs);
		MenuSupport(menu,WND_PLAQUETTE);
	}
	return(menu);
}
/* ========================================================================== */
void MenuItemDelete(Menu menu) {
	if(menu) {
		if(menu->items) {
			int i;
			for(i=0; i<menu->maxitems; i++) WndContextFree((menu->cdr)->f,menu->items[i].gc);
			free(menu->items); menu->items = 0;
		}
		menu->nbitems = menu->maxitems = 0;
	}
}
/* ========================================================================== */
void MenuDelete(Menu m) {
	// MenuItem *items; int i,nb;
	if(!m) return;
	// items = m->items; nb = m->maxitems;
	if(m->cdr) {
		int i;
		for(i=0; i<m->maxitems; i++) {
			// PRINT_GC(m->items[i].gc);
			WndContextFree((m->cdr)->f,m->items[i].gc); m->items[i].gc = 0;
			// PRINT_GC(m->items[i].gc);
		}
		OpiumDelete(m->cdr);
	}
	// for(i=0; i<nb; i++) PRINT_GC(items[i].gc);
	free(m);
	// for(i=0; i<nb; i++) PRINT_GC(items[i].gc);
}
/* ========================================================================== */
void OpiumMenuColorNb(Cadre cdr, Cadre fait, int *nc) {
	Menu menu,modele; int ic;

	if(!(menu = (Menu)cdr->adrs)) return;
	if(fait) modele = (Menu)fait->adrs; else modele = 0;
	if(modele) {
		menu->hilite = modele->hilite; menu->reverse = modele->reverse;
	} else {
		ic = *nc;
		menu->hilite = ic++; menu->reverse = ic++;
		*nc = ic;
	}
}
/* ========================================================================== */
void OpiumMenuColorSet(Cadre cdr, WndFrame f) {
	Menu menu;

	if(!(menu = (Menu)cdr->adrs)) return;
	WndFenColorSet(f,menu->hilite,WND_GC_HILITE);
	WndFenColorSet(f,menu->reverse,WND_GC_REVERSE);
}
/* ========================================================================== */
int OpiumSizeMenu(Cadre cdr, char from_wm) {
	MenuItem *item; Menu menu;
	int l,larg,haut,maxi,nbcols;
//	int nbitems;

	if(from_wm) {
#ifdef OPIUM_AJUSTE_WND
		larg = WndPixToCol(cdr->dh);
		cdr->dh = WndColToPix(larg);
		haut = WndPixToLig(cdr->dv);
		cdr->dv = WndLigToPix(haut);
#endif
		return(1);
	}
	if(DEBUG_OPIUM(1) && (cdr->type != OPIUM_MENU)) {
		WndPrint("!! Erreur systeme: %s(%s @%08X)\n",__func__,cdr->nom,(hexa)cdr->adrs);
		return(0);
	}
	menu = (Menu)cdr->adrs;
	if(!menu) return(0);
	item = menu->items; if(!item) return(0);
/*	nbitems = 0; while(item->texte) { nbitems++; item++; } */
	larg = 0; haut = 0;
	while(item->texte) {
		if(item->sel != MNU_ITEM_CACHE) {
			l = (int)strlen(item->traduit);
			if(l > larg) larg = l;
			haut++;
		}
		item++;
	}
	menu->nbitems = haut;
	menu->col = larg + 1;
	if(!menu->nbcols) {
		maxi = WndPixToLig(OpiumServerHeigth(0)) - 4;
		nbcols = ((haut - 1) / maxi) + 1;
	} else nbcols = menu->nbcols;
	menu->lig = ((haut - 1) / nbcols) + 1;
	menu->taille_calculee = 1;
	cdr->haut = WndLigToPix(menu->lig);
	cdr->larg = WndColToPix(larg + (menu->col * (nbcols - 1)));
	cdr->dh = cdr->larg; cdr->dv = cdr->haut;
	return(1);
}
/* ========================================================================== */
int OpiumRefreshMenu(Cadre cdr) {
	Menu menu; MenuItem *item;
	WndFrame f; WndContextPtr gc;
	int ligitem,colitem,lig,col,haut,numitem,h,l,marge,depart,demicol,n,aff;
	char doit_terminer;
//	int x,y,v;

	if(DEBUG_OPIUM(1) && (cdr->type != OPIUM_MENU)) {
		WndPrint("+++ %s sur le %s @%08X\n",__func__,cdr->nom,(hexa)cdr->adrs);
		return(0);
	}
	if(!OpiumCoordFenetre(cdr,&f)) return(0);
	
	menu = (Menu)cdr->adrs; if(!menu) return(0);
	item = menu->items; if(!item) return(0);
	if(menu->lig == 0) {
		if(!menu->taille_calculee) printf("(%s) Le menu %s n'a pas ete evalue\n",__func__,cdr->nom);
		return(0);
	}
	while(item->texte) { item->aff = -1; item++; }
	haut = WndLigToPix(1); demicol = WndColToPix(1) / 2;
	h = menu->col - 1;
	aff = 0;
	doit_terminer = WndRefreshBegin(f);
	numitem = cdr->ligmin; item = menu->items + numitem;
	while(item->texte) {
		if(item->sel == MNU_ITEM_CACHE) { item->aff = -1; goto suivant; }
		// colitem = numitem / menu->lig;
		// ligitem =  numitem - (colitem * menu->lig); lig = ligitem - cdr->ligmin;
		//- aff = numitem;
		colitem = aff / menu->lig; ligitem =  aff - (colitem * menu->lig);
		lig = aff - (colitem * menu->lig);
		if(lig >= cdr->lignb) goto suivant; // continue;
		col = (colitem * menu->col) - cdr->colmin;
		if(col >= cdr->colnb) break;
		else if((col + h) < 0 /* cdr->colmin */ ) goto suivant; // continue;
		item->aff = aff++;

		if(item->traduit) l = (int)strlen(item->traduit); else l = 0;
		if(item->type == MNU_CODE_BOOLEEN) {
			char *b;
			b = (char *)item->adrs;
			if(b) {
				if(*b) {
					if(!(gc = item->gc)) {
						OpiumDrawingRGB(cdr,&gc,0,0,0);
						MenuItemSetContext(menu,numitem+1,gc);
					}
					if(gc) WndContextBgndRGB(f,gc,GRF_RGB_YELLOW);
				} else {
					if((gc = item->gc)) {
						WndContextFree(f,gc);
						MenuItemSetContext(menu,numitem+1,0);
					}
				}
			}
		}
		if(!(gc = item->gc)) gc = MNU_NORMAL;
		if(cdr->planche && (cdr->support != WND_RIEN)) {
			if(ligitem) WndRelief(f,2,WND_RAINURE | WND_HORIZONTAL,WndColToPix(col),WndLigToPix(lig)-1,WndColToPix(h));
			if(colitem) WndRelief(f,2,WND_RAINURE | WND_VERTICAL,WndColToPix(col)-demicol,WndLigToPix(lig)-1,haut);
		} else WndClearText(f,gc,lig,col,menu->col);
		if(item->type == MNU_CODE_SEPARE) {
			if(cdr->planche) {
				if(l) {
					marge = (h - l) / 2;
					depart = marge + col;
					if(depart >= 0) WndWrite(f,gc,lig,depart,item->traduit);
					else if(-depart < l) WndWrite(f,gc,lig,0,item->traduit-depart);
				}
			} else if(l) {
				marge = (h - l) / 2;
				depart = marge + col;
				if(marge > 0) {
					if(col >= 0) WndHBar(f,gc,lig,col,marge);
					else if(-col < marge) WndHBar(f,gc,lig,0,marge + col);
				}
				if(depart >= 0) WndWrite(f,gc,lig,depart,item->traduit);
				else if(-depart < l) WndWrite(f,gc,lig,0,item->traduit-depart);
				n = h - (marge + l);
				if(n > 0) {
					depart = marge + l + col;
					if((depart + n) < cdr->colnb) WndHBar(f,gc,lig,depart,n);
					else WndHBar(f,gc,lig,depart,cdr->colnb-depart);
				}
			} else {
				if(col >= 0) WndHBar(f,gc,lig,col,menu->col);
				else if(-col < menu->col) WndHBar(f,gc,lig,0,menu->col+col);
			}
		} else if(menu->defaut == (numitem + 1)) {
			if(col >= 0) WndWrite(f,MNU_HILITE,lig,col,item->traduit);
			else if(-col < l) WndWrite(f,MNU_HILITE,lig,0,item->traduit-col);
		} else if(item->sel) {
			if(col >= 0) WndWrite(f,gc,lig,col,item->traduit);
			else if(-col < l) WndWrite(f,gc,lig,0,item->traduit-col);
		}
	suivant:
		numitem++; item++;
	}
#ifdef SUPERFLU
	if(cdr->planche && (cdr->support != WND_RIEN)) {
		if((aff == 1) && (menu->items[0].type == MNU_CODE_BOOLEEN) && (menu->items[0].adrs)) {
			if(*(char *)(menu->items[0].adrs)) WndEntoure(f,WND_RAINURES,0,0,cdr->larg,cdr->haut);
			else WndEntoure(f,WND_PLAQUETTE,0,0,cdr->larg,cdr->haut);
		} else WndEntoure(f,cdr->support,0,0,cdr->larg,cdr->haut);
	}
#endif
	if(doit_terminer) WndRefreshEnd(f);
	return(1);
}
/* ========================================================================== */
static void MenuPointe(Menu menu, int i, WndFrame f) {
	MenuItem *item; int lig,col,num,aff;

	if((i < 1) || (i > menu->nbitems)) return;
	num = i - 1;
	item = menu->items + num;
	// col = num / menu->lig; lig = num - (col * menu->lig);
	aff = item->aff;
	col = aff / menu->lig;
	lig = aff - (col * menu->lig);
	col *= menu->col;
	if(i == menu->item_pointe) {
		if(i == 0) return;
		if(menu->defaut == menu->item_pointe) {
			WndClearText(f,MNU_HILITE,lig,col,menu->col-1);
			WndWrite(f,MNU_HILITE,lig,col,item->traduit);
		} else {
			WndClearText(f,MNU_NORMAL,lig,col,menu->col-1);
			WndWrite(f,MNU_NORMAL,lig,col,item->traduit);
		}
		menu->item_pointe = 0;
	} else {
		WndClearText(f,MNU_REVERSE,lig,col,menu->col-1);
		WndWrite(f,MNU_REVERSE,lig,col,item->traduit);
		menu->item_pointe = i;
	}
}
/* ========================================================================== */
int OpiumRunMenu(Cadre cdr, WndUserRequest *e, int *cdr_ouverts) {
	MenuItem *item; Menu menu; Panel panel; Cadre cadre; WndFrame f;
	int lig,col; int defaut,aff_pointe,item_pointe,item_choisi,code_rendu;
	char eol,doit_terminer;
	int l,m,n; IntAdrs v; char *b;

	if(DEBUG_OPIUM(1) && (cdr->type != OPIUM_MENU)) {
		WndPrint("+++ %s sur le %s @%08X\n",__func__,cdr->nom,(hexa)cdr->adrs);
		return(0);
	}
	if(cdr->modexec == OPIUM_DISPLAY) return(0);
	if(!OpiumCoordFenetre(cdr,&f)) return(0);
	menu = (Menu)cdr->adrs;
	if(!menu->items) return(0);
//	cdr->ligmin = (cdr->dy)? WndPixToLig(cdr->dy - 1) + 1: 0;
//	cdr->colmin = (cdr->dx)? WndPixToCol(cdr->dx - 1) + 1: 0;
	lig = WndPixToLig(e->y) + cdr->ligmin;
	col = WndPixToCol(e->x) + cdr->colmin;
//	printf("(%s) evt en %d x %d, soit caractere pointe en col=%d, lig=%d\n",__func__,e->x,e->y,col,lig);

#ifdef WIN32_DEBUG	
	{	char tmp[80];
		sprintf(tmp, "lig:%d col:%d", lig, col);
		OpiumWin32Log(tmp);
	}
#endif

	// item_pointe = ((col / menu->col) * menu->lig) + lig + 1;
	aff_pointe = ((col / menu->col) * menu->lig) + lig;
	item_pointe = 0; while(menu->items[item_pointe].texte && (menu->items[item_pointe].aff != aff_pointe)) item_pointe++;
	item = menu->items + item_pointe; // printf("(%s.1) @%08X: %s.type=%d -> %08X\n",__func__,(hexa)item,item->texte,item->type,(hexa)(item->adrs));
	item_pointe += 1;
	defaut = menu->defaut;
	code_rendu = 0;
	item_choisi = 0;
	doit_terminer = 0;
	if(e->type == WND_KEY) {
		l = (int)strlen(e->texte);
		if(DEBUG_MENU(1)) {
			WndPrint("%04lX ( ",e->code);
			for(m=0; m<l; m++) WndPrint("%02X ",e->texte[m]); WndPrint(")\n");
		}
		if(l <= 0) {
			if(e->code == XK_Up) {
				if(menu->defaut) menu->defaut -= 1;
				while(menu->defaut) {
					item = menu->items + menu->defaut - 1;
					if((item->sel == MNU_ITEM_OK) && (item->aff >= 0)) break;
					menu->defaut -= 1;
				}
			} else if(e->code == XK_Down) do {
				item = menu->items + menu->defaut;
				menu->defaut += 1;
				if((item->sel == MNU_ITEM_OK) && (item->aff >= 0)) break;
				if(menu->defaut >= menu->maxitems) menu->defaut = 0;
			} while(menu->defaut);
			else if(e->code == XK_Home) menu->defaut = 0;
			else if(e->code == XK_KP_F4) code_rendu = menu->maxitems;
		} else if(e->code == XK_Alt_L) {
			if(e->texte[0] == 'w') return(1);
			if(e->texte[0] == 'q') { WndQueryExit = 1; return(1); }
		} else {
			eol = (e->texte[l - 1] == 0x0D);
			if(eol) e->texte[--l] = '\0';
			if(l > 0) {
				if((e->texte[0] == 0x7F) || (e->texte[0] == 0x08)) {
					if(menu->curs > 0) menu->curs -= 1;
					else menu->defaut = 0;
				} else {
					item = menu->items;
					m = 1; n = 0;
					while(item->texte) {
						if(!menu->curs) item->key = item->sel;
						if((item->key == MNU_ITEM_OK) && !strncasecmp(item->traduit + menu->curs,e->texte,l)) {
							if(!n) item_choisi = m;
							n++; 
						} else item->key = MNU_ITEM_HS;
						item++; m++;
					}
					if(n > 1) {
						menu->defaut = item_choisi;
						item_choisi = 0;
						menu->curs += 1;
					}
				}
			}
			if(eol && !item_choisi) item_choisi = menu->defaut;
		}

	} else if(e->type == WND_RELEASE) {
		if((item_pointe <= 0) || (item_pointe > menu->maxitems)) return(code_rendu);
		item = menu->items + item_pointe - 1;
		if(item->sel != MNU_ITEM_OK) {
			if(item->sel == MNU_ITEM_HS) OpiumFail("Desactive");
			return(code_rendu);
		}
		doit_terminer = WndRefreshBegin(f);
		MenuPointe(menu,menu->item_pointe,f);
		switch(e->code) {
		  case WND_MSELEFT:   item_choisi  = item_pointe; break;
		  case WND_MSEMIDDLE: menu->defaut = item_pointe; break;
		  default: /* autres entrees souris dans un menu: pas d'action */
			break;
		}

	} else if(e->type == WND_PRESS) switch(e->code) {
		case WND_MSELEFT:
		  if((item_pointe <= 0) || (item_pointe > menu->maxitems)) break;
		  item = menu->items + item_pointe - 1;
		  if(item->sel != MNU_ITEM_OK) {
			  if(item->sel == MNU_ITEM_HS) OpiumFail("Desactive");
			  break;
		  }
		  if(menu->item_pointe != item_pointe) {
			  doit_terminer = WndRefreshBegin(f);
			  MenuPointe(menu,menu->item_pointe,f);
			  MenuPointe(menu,item_pointe,f);
		  }
		  if((item->type == MNU_CODE_MENU) || (item->type == MNU_CODE_PANEL)) item_choisi = item_pointe;
		  break;
		default: /* autres selections souris dans un menu: pas d'action */
		  break;
	}

	if(defaut != menu->defaut) {
		if(defaut > 0) {
			item = menu->items + defaut - 1;
			doit_terminer = WndRefreshBegin(f);
			WndClearText(f,MNU_NORMAL,item->aff,0,cdr->larg);
			WndWrite(f,MNU_NORMAL,item->aff,0,item->traduit);
		}
		if(menu->defaut > 0) {
			item = menu->items + menu->defaut - 1;
			doit_terminer = WndRefreshBegin(f);
			WndClearText(f,MNU_HILITE,item->aff,0,cdr->larg);
			WndWrite(f,MNU_HILITE,item->aff,0,item->traduit);
		}
	}
	if(doit_terminer) WndRefreshEnd(f);
	if(item_choisi) {
		TypeMenuFctn fctnMenu;
		menu->curs = 0;
		item = menu->items + item_choisi - 1;
		OpiumNextTitle = item->traduit;
		if(cdr->log) LogBook(" (%s)\n",OpiumNextTitle);
		if(item->sel == MNU_ITEM_OK) switch(item->type) {
		  case MNU_CODE_MENU:
			if(!item->adrs) { code_rendu = 0; break; }
			menu = *(Menu *)item->adrs;
			code_rendu = OpiumUse(menu->cdr,OPIUM_EXEC);
			if(!code_rendu) *cdr_ouverts += 1;
			break;
		  case MNU_CODE_PANEL:
			if(!item->adrs) { code_rendu = 0; break; }
			panel = *(Panel *)item->adrs;
			if((panel->cdr)->f) OpiumUse(panel->cdr,OPIUM_EXEC);
			else {
				OpiumPosition(panel->cdr,cdr->x+e->x,cdr->y+e->y);
				code_rendu = OpiumUse(panel->cdr,OPIUM_EXEC);
				if(!code_rendu) *cdr_ouverts += 1;
			}
			break;
		  case MNU_CODE_EXEC:
			if(!item->adrs) { code_rendu = 0; break; }
			cadre = *(Cadre *)item->adrs;
			if(cadre->f) OpiumUse(cadre,OPIUM_EXEC);
			else {
				code_rendu = OpiumUse(cadre,OPIUM_EXEC);
				if(!code_rendu) *cdr_ouverts += 1;
			}
			break;
		  case MNU_CODE_FORK:
			if(!item->adrs) { code_rendu = 0; break; }
			cadre = *(Cadre *)item->adrs;
			if(cadre->f) OpiumUse(cadre,OPIUM_FORK);
			else {
				code_rendu = OpiumUse(cadre,OPIUM_FORK);
				if(!code_rendu) *cdr_ouverts += 1;
			}
			break;
		  case MNU_CODE_DISPLAY:
			if(!item->adrs) { code_rendu = 0; break; }
			cadre = *(Cadre *)item->adrs;
			OpiumUse(cadre,OPIUM_DISPLAY);
			code_rendu = 0;
			break;
		  case MNU_CODE_FONCTION:
			if(!item->adrs) { code_rendu = 0; break; }
			fctnMenu = (TypeMenuFctn)item->adrs;
			code_rendu = (*fctnMenu)(menu,item);
			break;
		  case MNU_CODE_COMMANDE:
			if(!item->adrs) { code_rendu = 0; break; }
			code_rendu = WndLaunch((char * )item->adrs);
			break;
		  case MNU_CODE_SEPARE:
			code_rendu = item_choisi;
			break;
		  case MNU_CODE_CONSTANTE:
			v = (IntAdrs)item->adrs;
			code_rendu = (int)v;
			break;
		  case MNU_CODE_BOOLEEN:
			b = (char *)item->adrs;
			if(b) {
				/* Cadre board; */
				*b = (*b)? 0: 1;
				if(*b) MenuItemAllume(menu,item_choisi,0,GRF_RGB_YELLOW); else MenuItemEteint(menu,item_choisi,0);
			}
			code_rendu = 0;
			break;
		  case MNU_CODE_RETOUR:
			code_rendu = item_choisi;
			break;
		}
		OpiumNextTitle = 0;
	}

	return(code_rendu);
}
#ifdef MENU_BARRE
/* ========================================================================== */
int MenuBarreManage(int menu_num, unsigned int item_num) {
	Menu menu; MenuItem *item; Panel panel; Cadre cadre;
	TypeFctn fctn;
	int code_rendu; char *b;

	code_rendu = 0;
	menu = MenuEnBarre; // GCC
	item = MenuEnBarre->items; while(item->texte && (item->aff != menu_num)) item++;
	if((item->type == MNU_CODE_MENU) && (item_num != 999)) {
		menu = *(Menu *)(item->adrs);
		item = menu->items; while(item->texte && (item->aff != item_num)) item++;
	}
	// printf("(%s) Menu #%d, item #%d soit texte=[%s] type %d ->  %08X\n",__func__,menu_num,item_num,item->texte,item->type,(hexa)item->adrs);
	OpiumNextTitle = item->traduit;
	if(item->sel == MNU_ITEM_OK) switch(item->type) {
	  case MNU_CODE_MENU:
		if(!item->adrs) break;
		menu = *(Menu *)item->adrs;
		code_rendu = OpiumUse(menu->cdr,OPIUM_EXEC);
		break;
	  case MNU_CODE_PANEL:
		if(!item->adrs) break;
		panel = *(Panel *)item->adrs;
		if((panel->cdr)->f) OpiumUse(panel->cdr,OPIUM_EXEC);
		else code_rendu = OpiumUse(panel->cdr,OPIUM_EXEC);
		break;
	  case MNU_CODE_EXEC:
		if(!item->adrs) break;
		cadre = *(Cadre *)item->adrs;
		if(cadre->f) OpiumUse(cadre,OPIUM_EXEC);
		else code_rendu = OpiumUse(cadre,OPIUM_EXEC);
		break;
	  case MNU_CODE_FORK:
		if(!item->adrs) break;
		cadre = *(Cadre *)item->adrs;
		if(cadre->f) OpiumUse(cadre,OPIUM_FORK);
		else code_rendu = OpiumUse(cadre,OPIUM_FORK);
		break;
	  case MNU_CODE_DISPLAY:
		if(!item->adrs) break;
		cadre = *(Cadre *)item->adrs;
		OpiumUse(cadre,OPIUM_DISPLAY);
		code_rendu = 0;
		break;
	  case MNU_CODE_FONCTION:
		if(!item->adrs) break;
		fctn = (TypeFctn)item->adrs;
		code_rendu = (*fctn)(menu,item);
		break;
	  case MNU_CODE_COMMANDE:
		if(!item->adrs) break;
		code_rendu = WndLaunch((char*)item->adrs);
		break;
	  case MNU_CODE_SEPARE:
		code_rendu = item_num + 1;
		break;
	  case MNU_CODE_CONSTANTE:
		code_rendu = (int)item->adrs;
		break;
	  case MNU_CODE_BOOLEEN:
		b = (char *)item->adrs;
		if(b) *b = (*b)? 0: 1;
		code_rendu = 0;
		break;
	  case MNU_CODE_RETOUR:
		code_rendu = item_num + 1;
		break;
	} else if(item->sel == MNU_ITEM_HS) OpiumFail("Desactive");
	OpiumNextTitle = 0;
	return(code_rendu);
}
/* ========================================================================== */
void MenuBarreCreate(Menu menu) {
	MenuItem *item;
	Menu sousmenu; MenuItem *sousitem;
	int col,lig;
	char *texte;

	MenuEnBarre = menu;
	item = menu->items; if(!item) return;
	col = 0;
	while(item->texte) {
		if(item->sel == MNU_ITEM_CACHE) goto item_svt;
		if(item->type == MNU_CODE_MENU) {
			if(!WndMenuCreate(item->traduit)) break;
			item->aff = col++;
			if(WndCodeHtml) goto item_svt;
			sousmenu = *(Menu *)item->adrs; if(!sousmenu) goto item_svt;  /* menu pas encore cree */
			sousitem = sousmenu->items; if(!sousitem) goto item_svt;      /* item vide */
			lig = 0;
			while(sousitem->texte) {
				/* (1) item pas montre => erreur sur le numero d'item rendu a MenuBarreManage;
				   (2) separation avec texte: trait de separation pas trace en plus du texte. */
				if(sousitem->sel == MNU_ITEM_CACHE) sousitem->aff = -1; else {
					switch(sousitem->type) {
					  case MNU_CODE_SEPARE: /* if(sousitem->traduit[0]) WndMenuItem(sousitem->traduit,'s',0); else */ WndMenuSepare(); break;
					  case MNU_CODE_RETOUR: break;
					  default: WndMenuItem(sousitem->traduit,
						(sousitem->type == MNU_CODE_MENU)? 'm': ((sousitem->type == MNU_CODE_PANEL)? 'p': '\0'),sousitem->sel); break;
					}
					sousitem->aff = lig++;
				}
				sousitem++;
			}
			WndMenuInsert();
		} else if((item->type != MNU_CODE_SEPARE) && (item->type != MNU_CODE_RETOUR)) {
			if(!WndMenuCreate(item->traduit)) break;
			item->aff = col++;
			if((item->type == MNU_CODE_FONCTION) || (item->type == MNU_CODE_COMMANDE)) 
				texte = L_("Lancer");
			else texte = L_("Afficher");
			WndMenuItem(texte,'p',item->sel); WndMenuInsert();
		}
item_svt:
		item++; // i++;
	}
	WndMenuCreate(L_("Fenetres")); WndMenuItem(L_("Sauver en JPEG"),0,1); WndMenuSepare(); WndMenuInsert();
	WndMenuDisplay();
}
//#define GESTION_BARRE_V0
/* ========================================================================== */
void MenuBarreFenetreOuverte(Cadre cdr) {
	WndMenuItem(cdr->titre,0,1);
}
/* ========================================================================== */
void MenuBarreExec() {
	Cadre cdr; WndUserRequest u; int cdr_ouverts; int i;

	WndQueryExit = 0;
	cdr = 0;
	cdr_ouverts = 0;
	u.type = -1; /* rapport au cas WND_PRESS */
#ifdef GESTION_BARRE_V0
	while(!WndQueryExit) {
		WndEventNew(&u,WND_WAIT);
		if(u.type == WND_BARRE) { if(u.x >= 0) MenuBarreManage(u.x,u.code); }
		else OpiumManage(0,&u,&cdr_ouverts);
		if(!WndMenuAffiche && !OpiumDisplayed(MenuEnBarre->cdr)) {
			if(!OpiumUse(MenuEnBarre->cdr,OPIUM_EXEC)) OpiumRefresh(MenuEnBarre->cdr);
		}
	}
#else
	// printf("(%s) Mode %s\n",__func__,WndCodeHtml? "serveur": "ecran");
	if(WndCodeHtml) {
		OpiumHttpDesc = 0;
		// printf("(%s) Sur le port %d\n",__func__,OpiumHttpPort);
		HttpServeur(OpiumHttpPort);
	} else while(!WndQueryExit) {
		if(WndEventNew(&u,WND_CHECK)) {
			// WndPrint("(%s) Recupere evt '%s %s'\n",__func__,(u.type==-1)?"neant":OpiumEventType[u.type],
			//	(u.type==-1)?"de chez  neant":((u.type == WND_KEY)?u.texte:OpiumMseButton[u.code]));
			OpiumDernierMessage[0] = '\0';
			OpiumManage(0,&u,&cdr_ouverts);
			if(!WndMenuAffiche && !OpiumDisplayed(MenuEnBarre->cdr)) {
				if(!OpiumUse(MenuEnBarre->cdr,OPIUM_EXEC)) OpiumRefresh(MenuEnBarre->cdr);
			}
			if(OpiumDernierMessage[0] == '!') OpiumWarn(OpiumDernierMessage);
			else if(OpiumDernierMessage[0] == '*') OpiumNote(OpiumDernierMessage+2);
		} else {
			if(OpiumUserPeriodic) (*OpiumUserPeriodic)();
			for(i=0; i<OpiumNbRefresh; i++) {
				cdr = OpiumToRefresh[i];
				if(cdr->restant > 0) cdr->restant -= 17;
				else {
					if(OpiumDisplayed(cdr)) {
						if(cdr->type == OPIUM_PANEL) PanelRefreshVars((Panel)(cdr->adrs));
						else if(cdr->type == OPIUM_INSTRUM) InstrumRefreshVar((Instrum)(cdr->adrs));
						else OpiumRefresh(cdr);
						cdr->restant = cdr->delai;
					}
				}
			}
			TimerMilliSleep(17);
		}
	}
#endif

}
#endif /* MENU_BARRE */

