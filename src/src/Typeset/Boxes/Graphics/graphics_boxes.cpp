
/******************************************************************************
* MODULE     : graphics.cpp
* DESCRIPTION: Boxes for graphics
* COPYRIGHT  : (C) 1999  Joris van der Hoeven
*******************************************************************************
* This software falls under the GNU general public license and comes WITHOUT
* ANY WARRANTY WHATSOEVER. See the file $TEXMACS_PATH/LICENSE for more details.
* If you don't have this file, write to the Free Software Foundation, Inc.,
* 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
******************************************************************************/

#include "env.hpp"
#include "Boxes/graphics.hpp"
#include "Boxes/composite.hpp"
#include "Graphics/math_util.hpp"

/******************************************************************************
* Graphics boxes
******************************************************************************/

struct graphics_box_rep: public composite_box_rep {
  frame f;
  grid g;
  point lim1, lim2;
  SI old_clip_x1, old_clip_x2, old_clip_y1, old_clip_y2;
  graphics_box_rep (
    path ip, array<box> bs, frame f, grid g, point lim1, point lim2);
  frame get_frame ();
  grid get_grid ();
  void  get_limits (point& lim1, point& lim2);
  operator tree () { return "graphics"; }
  void pre_display (ps_device &dev);
  void post_display (ps_device &dev);
  int reindex (int i, int item, int n);
  gr_selections graphical_select (SI x1, SI y1, SI x2, SI y2);
};

graphics_box_rep::graphics_box_rep (
  path ip2, array<box> bs2, frame f2, grid g2, point lim1b, point lim2b):
  composite_box_rep (ip2, bs2), f (f2), g (g2), lim1 (lim1b), lim2 (lim2b)
{
  point flim1= f(lim1), flim2= f(lim2);
  x1= (SI) min (flim1[0], flim2[0]);
  y1= (SI) min (flim1[1], flim2[1]);
  x2= (SI) max (flim1[0], flim2[0]);
  y2= (SI) max (flim1[1], flim2[1]);
  finalize ();
}

frame
graphics_box_rep::get_frame () {
  return f;
}

grid
graphics_box_rep::get_grid () {
  return g;
}

void
graphics_box_rep::get_limits (point& lim1b, point& lim2b) {
  lim1b= lim1;
  lim2b= lim2;
}

void
graphics_box_rep::pre_display (ps_device &dev) {
  dev->get_clipping (old_clip_x1, old_clip_y1, old_clip_x2, old_clip_y2);
  dev->extra_clipping (x1, y1, x2, y2);
}

void
graphics_box_rep::post_display (ps_device &dev) {
  dev->set_clipping (
    old_clip_x1, old_clip_y1, old_clip_x2, old_clip_y2, true);
}

int
graphics_box_rep::reindex (int i, int item, int n) {
  return i;
}

gr_selections
graphics_box_rep::graphical_select (SI x1, SI y1, SI x2, SI y2) {
  gr_selections res;
  int i, n= subnr();
  for (i=0; i<n; i++)
    res << bs[i]->graphical_select (x1- sx(i), y1- sy(i),
                                    x2- sx(i), y2- sy(i));
  return res;
}
/*NOTE: It seems that the dimensions of the boxes that inherit from
  composite_box are not calculated correctly (namely : one can find
  points inside the box that are outside the rectangle (x1, y1, x2, y2)
  that defines the border of the box). As a consequence, we use a traversal
  routine that doesn't tests contains_rectangle(). When this problem
  will have been corrected, the method of composite_box should work,
  and consequently, its more specific implementation above should be
  removed (this is the same in concat_boxes and stack_boxes). */

/******************************************************************************
* Group boxes
******************************************************************************/

struct graphics_group_box_rep: public composite_box_rep {
  graphics_group_box_rep (path ip, array<box> bs):
    composite_box_rep (ip, bs, true) { finalize (); }
  bool access_allowed () { return false; }
  operator tree () { return "graphics_group"; }
  path find_lip () { return path (-1); }
  path find_rip () { return path (-1); }
  gr_selections graphical_select (SI x, SI y, SI dist);
  gr_selections graphical_select (SI x1, SI y1, SI x2, SI y2);
  int reindex (int i, int item, int n);
};

gr_selections
graphics_group_box_rep::graphical_select (SI x, SI y, SI dist) {
  gr_selections res;
  if (graphical_distance (x, y) <= dist) {
    gr_selection gs;
    gs->dist= graphical_distance (x, y);
    gs->cp << reverse (path (0, ip));
    res << gs;
  }
  return res;
}

gr_selections
graphics_group_box_rep::graphical_select (SI x1, SI y1, SI x2, SI y2) {
  gr_selections res;
  if (in_rectangle (x1, y1, x2, y2)) {
    gr_selection gs;
    gs->dist= graphical_distance (x1, y1);
    gs->cp << reverse (path (0, ip));
    res << gs;
  }
  return res;
}

int
graphics_group_box_rep::reindex (int i, int item, int n) {
  return i;
}

/******************************************************************************
* Point boxes
******************************************************************************/

struct point_box_rep: public box_rep {
  point p;
  SI r;
  color col;
  int fill;
  color fill_col;
  string style;
  point_box_rep (
    path ip, point p, SI radius, color col,
    int fill, color fill_col, string style);
  SI graphical_distance (SI x, SI y) { return (SI)norm (p - point (x, y)); }
  void display (ps_device dev);
  operator tree () { return "point"; }
};

point_box_rep::point_box_rep (
  path ip2, point p2, SI r2, color col2,
  int fill2, color fill_col2, string style2):
    box_rep (ip2), p (p2), r (r2), col (col2),
    fill (fill2), fill_col (fill_col2), style (style2)
{
  x1= x3= ((SI) p[0]) - r;
  y1= y3= ((SI) p[1]) - r;
  x2= x4= ((SI) p[0]) + r;
  y2= y4= ((SI) p[1]) + r;
}

void
point_box_rep::display (ps_device dev) {
  array<SI> x (4), y (4);
  x[0]= ((SI) p[0]) - r;
  y[0]= ((SI) p[1]) - r;
  x[1]= ((SI) p[0]) - r;
  y[1]= ((SI) p[1]) + r;
  x[2]= ((SI) p[0]) + r;
  y[2]= ((SI) p[1]) + r;
  x[3]= ((SI) p[0]) + r;
  y[3]= ((SI) p[1]) - r;
  dev->set_line_style (PIXEL);
  if (style == "square") {
    if (fill == FILL_MODE_INSIDE || fill == FILL_MODE_BOTH) {
      dev->set_color (fill_col);
      dev->line (x[0], y[0], x[1], y[1]);
      dev->line (x[1], y[1], x[2], y[2]);
      dev->line (x[2], y[2], x[3], y[3]);
      dev->line (x[3], y[3], x[0], y[0]);
      dev->polygon (x, y, false);
    }
    if (fill == FILL_MODE_NONE || fill == FILL_MODE_BOTH) {
      dev->set_color (col);
      dev->line (x[0], y[0], x[1], y[1]);
      dev->line (x[1], y[1], x[2], y[2]);
      dev->line (x[2], y[2], x[3], y[3]);
      dev->line (x[3], y[3], x[0], y[0]);
    }
  }
  else {
    if (style == "disk"
     || fill == FILL_MODE_INSIDE || fill == FILL_MODE_BOTH) {
      dev->set_color (style == "disk" ? col : fill_col);
      dev->arc (x[0], y[0]+dev->pixel, x[2], y[2]+dev->pixel, 0, 64*360);
      dev->fill_arc (x[0], y[0]+dev->pixel, x[2], y[2]+dev->pixel, 0, 64*360);
    }
    if (fill == FILL_MODE_NONE || fill == FILL_MODE_BOTH) {
      dev->set_color (col);
      dev->arc (x[0], y[0]+dev->pixel, x[2], y[2]+dev->pixel, 0, 64*360);
    }
  }
}

/******************************************************************************
* Curve boxes
******************************************************************************/

struct curve_box_rep: public box_rep {
  array<point> a;
  SI width;
  color col;
  curve c;
  array<bool> style;
  SI style_unit;
  array<SI> styled_n;
  int fill;
  color fill_col;
  array<box> arrows;
  curve_box_rep (path ip, curve c, SI width, color col,
		 array<bool> style, SI style_unit,
		 int fill, color fill_col,
		 array<box> arrows);
  box transform (frame fr);
  SI graphical_distance (SI x, SI y);
  gr_selections graphical_select (SI x, SI y, SI dist);
  gr_selections graphical_select (SI x1, SI y1, SI x2, SI y2);
  void display (ps_device dev);
  operator tree () { return "curve"; }
  SI length ();
  void apply_style ();
};

curve_box_rep::curve_box_rep (path ip2, curve c2, SI W, color C,
  array<bool> style2, SI style_unit2, int fill2, color fill_col2,
  array<box> arrows2)
  :
  box_rep (ip2), width (W), col (C), c (c2),
  style (style2), style_unit (style_unit2),
  fill (fill2), fill_col (fill_col2)
{
  a= c->rectify (PIXEL);
  int i, n= N(a);
  x1= y1= x3= y3= MAX_SI;
  x2= y2= x4= y4= -MAX_SI;
  for (i=0; i<(n-1); i++) {
    x1= min (x1, min ((SI) a[i][0], (SI) a[i+1][0]));
    y1= min (y1, min ((SI) a[i][1], (SI) a[i+1][1]));
    x2= max (x2, max ((SI) a[i][0], (SI) a[i+1][0]));
    y2= max (y2, max ((SI) a[i][1], (SI) a[i+1][1]));
  }
  apply_style ();
  arrows= array<box>(2);
  point p1, p2;
  bool error;
  if (N(arrows2)>0 && !nil (arrows2[0])) {
    point tg= c->grad (0.0, error);
    if (!error) {
      frame fr= scaling (1.0, a[0]) *
	        rotation_2D (point (0.0, 0.0), arg (tg));
      arrows[0]= arrows2[0]->transform (fr);
      if (!nil (arrows[0])) {
        x1= min (x1, arrows[0]->x1);
        y1= min (y1, arrows[0]->y1);
        x2= max (x2, arrows[0]->x2);
        y2= max (y2, arrows[0]->y2);
      }
    }
  }
  if (N(arrows2)>1 && !nil (arrows2[1])) {
    point tg= c->grad (1.0, error);
    if (!error) {
      frame fr= scaling (1.0, a[N(a)-1]) *
	        rotation_2D (point (0.0, 0.0), arg (tg));
      arrows[1]= arrows2[1]->transform (fr);
      if (!nil (arrows[1])) {
        x1= min (x1, arrows[1]->x1);
        y1= min (y1, arrows[1]->y1);
        x2= max (x2, arrows[1]->x2);
        y2= max (y2, arrows[1]->y2);
      }
    }
  }
  x3= x1 - (width>>1); y3= y1 - (width>>1); 
  x4= x2 + (width>>1); y4= y2 + (width>>1);
}

box
curve_box_rep::transform (frame fr) {
  return curve_box (ip, fr (c), width, col,
    style, style_unit, fill, fill_col, arrows);
}

SI
curve_box_rep::graphical_distance (SI x, SI y) {
  SI gd= MAX_SI;
  point p (x, y);
  int i;
  for (i=0; i<N(a)-1; i++) {
    axis ax;
    ax.p0= a[i];
    ax.p1= a[i+1];
    gd= min (gd, (SI)seg_dist (ax, p));
  }
  return gd;
}

gr_selections
curve_box_rep::graphical_select (SI x, SI y, SI dist) {
  gr_selections res;
  if (graphical_distance (x, y) <= dist) {
    array<double> abs;
    array<point> pts;
    array<path> paths;
    int np= c->get_control_points (abs, pts, paths);
    point p (x, y);
    int i;
    for (i=0; i<N(pts); i++) {
      SI n= (SI)norm (p - pts[i]);
      if (n <= dist) {
        gr_selection gs;
        gs->dist= n;
        gs->cp << reverse (paths[i]);
        res << gs;
      }
    }
    if (N(res) != 0) return res;
    int ne= np-1;
    if (np>1 && (abs[0]!=0.0 || abs[np-1]!=1.0))
      ne++;
    for (i=0; i<ne; i++) {
      bool b;
      double t= c->find_closest_point (abs[i], abs[(i+1)%np], p, PIXEL, b);
      if (b) {
        point p2= c->evaluate (t);
        SI n= (SI)norm (p - p2);
        if (n <= dist) {
          gr_selection gs;
          gs->dist= n;
          gs->cp << reverse (paths[i]);
          gs->cp << reverse (paths[(i+1)%np]);
          res << gs;
        }
      }
    }
  }
  return res;
}

gr_selections
curve_box_rep::graphical_select (SI x1, SI y1, SI x2, SI y2) {
  gr_selections res;
  if (in_rectangle (x1, y1, x2, y2)) {
    gr_selection gs;
    gs->dist= graphical_distance (x1, y1);
    gs->cp << reverse (path (0, ip));
    res << gs;
  }
  return res;
}

void
curve_box_rep::display (ps_device dev) {
  int i, n;
  if (fill == FILL_MODE_INSIDE || fill == FILL_MODE_BOTH) {
    dev->set_color (fill_col);
    n= N(a);
    array<SI> x (n), y (n);
    for (i=0; i<n; i++) {
      x[i]= (SI)a[i][0];
      y[i]= (SI)a[i][1];
    }
    dev->polygon (x, y, false);
  }
  if (fill == FILL_MODE_NONE || fill == FILL_MODE_BOTH) {
    dev->set_color (col);
    dev->set_line_style (width, 0, false);
    if (N (style) == 0) {
      n= N(a);
      for (i=0; i<(n-1); i++)
        dev->line ((SI) a[i][0], (SI) a[i][1], (SI) a[i+1][0], (SI) a[i+1][1]);
    }
    else {
      SI li=0, o=0;
      i=0;
      int no;
      point prec= a[0];
      for (no=0; no<N(styled_n); no++) {
        point seg= a[i+1]-a[i];
        while (fnull (norm(seg),1e-6) && i+2<N(a)) {
          i++;
          seg= a[i+1]-a[i];
        }
        if (fnull (norm(seg),1e-6) && i+2>=N(a))
          break;
        SI lno= styled_n[no]*style_unit,
           len= li+(SI)norm(seg);
        while (i+2<N(a) && lno>len) {
          li= len;
          if (no%2!=0) {
         // 1st subsegment of a dash
            dev->line ((SI) prec[0], (SI) prec[1],
                       (SI) a[i+1][0], (SI) a[i+1][1]);
  	    prec= a[i+1];
          }
          i++;
          seg= a[i+1]-a[i];
          len= li+(SI)norm(seg);
        }
        o= lno-li;
     /* We could also use this one in order to use lines with
        round ends. But it doesn't work well when the width
        of the line becomes bigger than the style unit length.
        Anyway (although I don't know if there is a way to do
        lines with round ends in PostScript), our current PostScript
        output in GhostView uses square line ends, so we do the same.
        SI inc= ((no%2==0?1:-1) * width)/2;
        point b= a[i] + (o+inc)*(seg/norm(seg)); */
        if (i<N(a)) {
          point b= a[i] + o*(seg/norm(seg));
          if (no%2==0)
            prec= b;
          else
         // Last subsegment of a dash
            dev->line ((SI) prec[0], (SI) prec[1], (SI) b[0], (SI) b[1]);
         // TODO: Use XDrawLines() and the join style to draw correctly
         //   the subsegments ; implement this for Postscript as well.
        }
      }
    }
  }

  rectangles ll;
  if (!nil (arrows[0])) arrows[0]->redraw (dev, path (), ll);
  if (!nil (arrows[1])) arrows[1]->redraw (dev, path (), ll);
}

SI
curve_box_rep::length () {
  int i, n= N(a);
  SI res= 0;
  for (i=1; i<n; i++)
    res+= (SI)norm (a[i] - a[i-1]);
  return res;
}

void
curve_box_rep::apply_style () {
  int n= N(style);
  if (n<=0 || fnull (style_unit,1e-6)) return;
  int i;
  bool all0=true, all1=true;
  for (i=0; i<n; i++) {
    if (style[i]) all0= false;
    if (!style[i]) all1= false;
  }
  if (all1) style= array<bool>(0);
  if (all0 || all1) return;

  int n2= 0;
  i= 0;
  while (!style[i]) i++;
  while (i<N(style)) {
    if (style[i]) n2++; else break;
    i++;
  }

  if (a[0]==a[N(a)-1]) n2= n; // Closed curves

  SI l= length (), l1= n*style_unit, n1= l/l1 + 1, l2= n2*style_unit;
  l1= (SI)((((double)l)*((double)l1)) / ((double)(n1*l1 + l2)));
  style_unit= l1/n;
  l2= n2*style_unit;

  int nfrag=0, prevfrag=-1;
  for (i=0; i<n; i++) {
    int frag= style[i]?1:0;
    if (frag!=prevfrag && frag==1) nfrag++;
    prevfrag= frag;
  }

  int nfrag2=0;
  prevfrag=-1;
  for (i=0; i<n2; i++) {
    int frag= style[i]?1:0;
    if (frag!=prevfrag && frag==1) nfrag2++;
    prevfrag= frag;
  }

  bool common_frag= style[0] && style[n-1] && n1>1;
  if (common_frag) nfrag--;
  styled_n= array<SI>(2*(nfrag*n1 + nfrag2));

  int no=0, nbu=0;
  prevfrag=-1;
  for (i=0; i<n1+1; i++) {
    int j;
    for (j=0; j<(i==n1?n2:n); j++) {
      int frag= style[j]?1:0;
      if (frag!=prevfrag) {
        if (frag==1) {
          styled_n[no]= nbu;
          no++;
        }
        else
        if (frag==0 && prevfrag!=-1) {
          styled_n[no]= nbu;
          no++;
        }
      }
      prevfrag= frag;
      nbu++;
    }
  }
  if (style[n2-1]) styled_n[N(styled_n)-1]= nbu;
}

/******************************************************************************
* User interface
******************************************************************************/

box
graphics_box (
  path ip, array<box> bs, frame f, grid g, point lim1, point lim2)
{
  return new graphics_box_rep (ip, bs, f, g, lim1, lim2);
}

box
graphics_group_box (path ip, array<box> bs) {
  return new graphics_group_box_rep (ip, bs);
}

box
point_box (
  path ip, point p, SI r, color col, int fill, color fill_col, string style) {
  return new point_box_rep (ip, p, r, col, fill, fill_col, style);
}

box
curve_box (path ip, curve c, SI width, color col,
  array<bool> style, SI style_unit,
  int fill, color fill_col,
  array<box> arrows)
{
  return new curve_box_rep (ip, c, width, col,
			    style, style_unit, fill, fill_col, arrows);
}
