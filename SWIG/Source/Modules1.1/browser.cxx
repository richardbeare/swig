/* ----------------------------------------------------------------------------- 
 * browser.cxx
 *
 *     A web-base parse tree browser using SWILL.   This is an optional
 *     feature that's normally disabled.
 * 
 * Author(s) : David Beazley (beazley@cs.uchicago.edu)
 *
 * Copyright (C) 2002.  The University of Chicago
 * See the file LICENSE for information on usage and redistribution.	
 * ----------------------------------------------------------------------------- */

static char cvsroot[] = "$Header$";

#include "swig11.h"

#ifdef SWIG_SWILL
extern "C" {
#include "swill.h"
}

static FILE *out = 0;
static Node *view_top = 0;
      
class Browser : public Dispatcher {
  void show_checkbox(Node *t, Node *n) {
    int v = 0;
    if (Getmeta(n,"visible")) {
      v = 1;
    }
    if (v) {
      Printf(out,"<a name=\"n%x\"></a>[<a href=\"hide.html?node=0x%x&hn=0x%x#n%x\">-</a>] ",  n, t, n,n);
    } else {
      Printf(out,"<a name=\"n%x\"></a>[<a href=\"show.html?node=0x%x&hn=0x%x#n%x\">+</a>] ",  n, t, n,n);
    }
  }
  void show_attributes(Node *obj) {
    if (!Getmeta(obj,"visible")) return;
    String *os = NewString("");
    String *k;
    k = Firstkey(obj);
    while (k) {
      if ((Cmp(k,"nodeType") == 0) || (Cmp(k,"firstChild") == 0) || (Cmp(k,"lastChild") == 0) ||
	  (Cmp(k,"parentNode") == 0) || (Cmp(k,"nextSibling") == 0) || 
	  (Cmp(k,"previousSibling") == 0) || (*(Char(k)) == '$')) {
	/* Do nothing */
      } else if (Cmp(k,"parms") == 0) {
	Printf(os,"<a href=\"data.html?n=0x%x\">?</a> %-12s - %s\n", Getattr(obj,k), k, ParmList_protostr(Getattr(obj,k)));
      } else {
	DOH *o;
	char *trunc = "";
	if (DohIsString(Getattr(obj,k))) {
	  o = Str(Getattr(obj,k));
	  if (Len(o) > 70) {
	    trunc = "...";
	  }
	  Replaceall(o,"<","&lt;");
	  Printf(os,"<a href=\"data.html?n=0x%x\">?</a> %-12s - \"%(escape)-0.70s%s\"\n", Getattr(obj,k), k, o, trunc);
	  Delete(o);
	} else {
	  Printf(os,"<a href=\"data.html?n=0x%x\">?</a> %-12s - 0x%x\n", Getattr(obj,k), k, Getattr(obj,k));
	}
      }
      k = Nextkey(obj);
    }
    Printf(out,"<FONT color=\"#660000\"><pre>\n%s</pre></FONT>\n", Char(os));
    Delete(os);
  }

public:
  virtual int emit_one(Node *n) {
    char *tag = Char(nodeType(n));
    char *file = Char(Getfile(n));
    int   line = Getline(n);
    char *name = GetChar(n,"name");

    show_checkbox(view_top, n);
    Printf(out,"<b><a href=\"index.html?node=0x%x\">%s</a></b>", n, tag);
    if (name) {
      Printf(out," (%s)", name);
    }
    Printf(out,".  %s:%d\n", file, line);
    Printf(out,"<br>");
    Dispatcher::emit_one(n);
    return SWIG_OK;
  }
  virtual int emit_children(Node *n) {
    if (Getmeta(n,"visible")) {
      Printf(out,"<blockquote>\n");
      Dispatcher::emit_children(n);
      Printf(out,"</blockquote>\n");
    }
    return SWIG_OK;
  }
  virtual int defaultHandler(Node *n) {
    show_attributes(n);
    return SWIG_OK;
  }
  virtual int top(Node *n) {
    show_attributes(n);
    emit_children(n);
    return SWIG_OK;
  }
  virtual int includeDirective(Node *n) {
    show_attributes(n);
    emit_children(n);
    return SWIG_OK;
  }
  virtual int importDirective(Node *n) {
    show_attributes(n);
    emit_children(n);
    return SWIG_OK;
  }

  virtual int addmethodsDirective(Node *n) {
    show_attributes(n);
    emit_children(n);
    return SWIG_OK;
  }
  virtual int classDeclaration(Node *n) {
    show_attributes(n);
    emit_children(n);
    return SWIG_OK;
  }

  virtual int templateDeclaration(Node *n) {
    show_attributes(n);
    emit_children(n);
    return SWIG_OK;
  }

  virtual int enumDeclaration(Node *n) {
    show_attributes(n);
    emit_children(n);
    return SWIG_OK;
  }
  virtual int typemapDirective(Node *n) {
    show_attributes(n);
    emit_children(n);
    return SWIG_OK;
  }
  virtual int namespaceDeclaration(Node *n) {
    show_attributes(n);
    emit_children(n);
    return SWIG_OK;
  }

};

static int browser_exit = 0;
static Node *tree_top = 0;
static Browser *browse = 0;

/* ----------------------------------------------------------------------
 * exit_handler()      - Force the browser to exit
 * ---------------------------------------------------------------------- */

void exit_handler(FILE *f) {
  browser_exit = 1;
  Printf(f,"Terminated.\n");
}

/* ----------------------------------------------------------------------
 * node_handler()      - Generate information about a specific node
 * ---------------------------------------------------------------------- */

static void display(FILE *f, Node *n) {
  /* Print standard HTML header */
  
  Printf(f,"<HTML><HEAD><TITLE>SWIG-%s</TITLE></HEAD><BODY BGCOLOR=\"#ffffff\">\n", SWIG_VERSION); 
  Printf(f,"<b>SWIG-%s</b><br>\n", SWIG_VERSION);
  Printf(f,"[ <a href=\"exit.html\">Exit</a> ]");
  Printf(f," [ <a href=\"index.html?node=0x%x\">Top</a> ]", tree_top);
  if (n != tree_top) {
    Printf(f," [ <a href=\"index.html?node=0x%x\">Up</a> ]", parentNode(n));
  }
  Printf(f,"<br><hr><p>\n");

  out = f;

  browse->emit_one(n);

  /* Print standard footer */
  Printf(f,"<br><hr></BODY></HTML>\n");

}

void node_handler(FILE *f) {
  Node *n = 0;
  if (!swill_getargs("p(node)", &n)) {
    n = tree_top;
  }
  view_top = n;
  display(f,n);
}


/* ----------------------------------------------------------------------
 * hide_handler()      - Hide a node 
 * ---------------------------------------------------------------------- */

void hide_handler(FILE *f) {
  Node *n = 0;
  if (!swill_getargs("p(hn)", &n)) {
    n = 0;
  }
  if (n) {
    Delmeta(n,"visible");
  }
  node_handler(f);
}

void show_handler(FILE *f) {
  Node *n = 0;
  if (!swill_getargs("p(hn)", &n)) {
    n = 0;
  }
  if (n) {
    Setmeta(n,"visible","1");
  }
  node_handler(f);
}

void raw_data(FILE *out, Node *obj) {
  if (!obj) return;
  if (DohIsMapping(obj)) {
    String *k;
    String *os = NewString("");
    Printf(os,"Hash {\n");
    k = Firstkey(obj);
    while (k) {
      DOH *o;
      const char *trunc = "";
      if (DohIsString(Getattr(obj,k))) {
	o = Str(Getattr(obj,k));
	if (Len(o) > 70) {
	  trunc = "...";
	}
	Replaceall(o,"<","&lt;");
	Printf(os,"    <a href=\"data.html?n=0x%x\">?</a> %-12s - \"%(escape)-0.70s%s\"\n", Getattr(obj,k), k, o, trunc);
	Delete(o);
      } else {
	Printf(os,"    <a href=\"data.html?n=0x%x\">?</a> %-12s - 0x%x\n", Getattr(obj,k), k, Getattr(obj,k));
      }
      k = Nextkey(obj);
    }
    Printf(os,"}\n");
    Printf(out,"<FONT color=\"#660000\"><pre>\n%s</pre></FONT>\n", Char(os));
    Delete(os);
  } else if (DohIsString(obj)) {
    String *o = Str(obj);
    Replaceall(o,"<","&lt;");
    Printf(out,"<FONT color=\"#660000\"><pre>\n%s</pre></FONT>\n", Char(o));
    Delete(o);
  } else if (DohIsSequence(obj)) {
    int i;
    String *os = NewString("");
    Printf(os,"List [\n");
    for (i = 0; i < Len(obj); i++) {
      DOH *o = Getitem(obj,i);
      const char *trunc = "";
      if (DohIsString(o)) {
	String *s = Str(o);
	if (Len(s) > 70) {
	  trunc = "...";
	}
	Replaceall(o,"<","&lt;");
	Printf(os,"    <a href=\"data.html?n=0x%x\">?</a> [%d] - \"%(escape)-0.70s%s\"\n", o,i,s, trunc);
	Delete(s);
      } else {
	Printf(os,"    <a href=\"data.html?n=0x%x\">?</a> [%d] - 0x%x\n", o, i, o);
      }
    }
    Printf(os,"\n]\n");
    Printf(out,"<FONT color=\"#660000\"><pre>\n%s</pre></FONT>\n", Char(os));
    Delete(os);
  }
}

void data_handler(FILE *f) {
  DOH *n = 0;
  if (!swill_getargs("p(n)", &n)) {
    n = 0;
  }
  Printf(f,"<HTML><HEAD><TITLE>SWIG-%s</TITLE></HEAD><BODY BGCOLOR=\"#ffffff\">\n", SWIG_VERSION); 
  Printf(f,"<b>SWIG-%s</b><br>\n", SWIG_VERSION);
  Printf(f,"[ <a href=\"exit.html\">Exit</a> ]");
  Printf(f," [ <a href=\"index.html?node=0x%x\">Top</a> ]", tree_top);
  Printf(f,"<br><hr><p>\n");
  if (n) {
    raw_data(f,n);
  }
  /* Print standard footer */
  Printf(f,"<br><hr></BODY></HTML>\n");
}

void symbol_handler(FILE *f) {
  Symtab *sym;
  char   *name = 0;

  Printf(f,"<HTML><HEAD><TITLE>SWIG-%s</TITLE></HEAD><BODY BGCOLOR=\"#ffffff\">\n", SWIG_VERSION); 
  Printf(f,"<b>SWIG-%s</b><br>\n", SWIG_VERSION);
  Printf(f,"[ <a href=\"exit.html\">Exit</a> ]");
  Printf(f," [ <a href=\"index.html?node=0x%x\">Top</a> ]", tree_top);
  Printf(f,"<br><hr><p>\n");
  
  if (!swill_getargs("p(sym)|s(name)", &sym, &name)) {
    sym = Swig_symbol_getscope("");
    name = 0;
  }
  if (!sym) {
    Printf(f,"No symbol table specified!\n");
    return;
  }
  {
    String *q = Swig_symbol_qualifiedscopename(sym);
    if (!Len(q)) {
      Printf(f,"<b>Symbol table: :: (global)</b><br>\n"); 
    } else {
      Printf(f,"<b>Symbol table: %s</b><br>\n", q);
    }
    Delete(q);
  }
  
  fprintf(f,"<p><form action=\"symbol.html\" method=GET>\n");
  fprintf(f,"Symbol lookup: <input type=text name=name size=40></input><br>\n");
  fprintf(f,"<input type=hidden name=sym value=\"0x%x\">\n", sym);
  fprintf(f,"Submit : <input type=submit></input>\n");
  fprintf(f,"</form>");

  if (name) {
    Node *n = Swig_symbol_clookup(name,sym);
    Printf(f,"Symbol '%s':\n", name);
    Printf(f,"<blockquote>\n");
    if (!n) {
      Printf(f,"Not defined!\n");
    } else {
      raw_data(f,n);
    }
    Printf(f,"</blockquote>\n");
  }

  Printf(f,"<p><b>Nested scopes</b><br>\n");
  Printf(f,"<blockquote><pre>\n");
  {
    String *key;
    Hash   *h;
    Node   *n;
    h = firstChild(sym);
    while (h) {
      Printf(f,"<a href=\"symbol.html?sym=0x%x\">%s</a>\n", h, Getattr(h,"name"));
      h = nextSibling(h);
    }
  }
  Printf(f,"</pre></blockquote>\n");
  
  Printf(f,"<p><b>Symbol table contents</b></br>\n");
  raw_data(f,Getattr(sym,"symtab"));
  Printf(f,"<br><hr></BODY></HTML>\n");

}
#endif

void
Swig_browser(Node *top, int port) {
#ifdef SWIG_SWILL
  int sport;
  browser_exit = 0;

  /* Initialize the server */
  sport = swill_init(port);
  if (sport < 0) {
    Printf(stderr,"Couldn't open socket on port %d. Sorry.\n", port);
    return;
  }
  browse = new Browser();
  Setmeta(top,"visible","1");
  tree_top = top;

  Printf(stderr,"SWIG: Tree browser listening on port %d\n", sport);

  swill_handle("exit.html", exit_handler,0);
  swill_handle("index.html", node_handler, 0);
  swill_handle("hide.html", hide_handler,0);
  swill_handle("show.html", show_handler,0);
  swill_handle("data.html", data_handler,0);
  swill_handle("symbol.html", symbol_handler, 0);
  swill_netscape("index.html");

  while (!browser_exit) {
    swill_serve();
  }
  Printf(stderr,"Browser terminated.\n");
  swill_close();
  delete browse;
  return;
#endif
}





