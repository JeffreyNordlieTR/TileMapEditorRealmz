#include "MapUI.h"
#include "MapResources.h"
#include "AuxUI.h"
#include "DrawingFunctions.h"
#include "DrawingToolUI.h"
#include "CtrlMap.h"
#include "Thing.h"
#ifdef _WIN32
#include <Windows.h>
#endif

extern data::MapResources* gResources;
extern ui::AuxUI* gAuxUI;
extern ui::DrawingToolUI* gDrawingToolUI;
extern ctrl::CtrlMap* ctrlMap;

namespace GtkUserInterface { extern GtkBuilder* builder; }

GdkPixbuf* ui::MapUI::_pixelbuf_full_Grid = NULL;

ui::MapUI::MapUI(std::string name, int width, int height) : Map(name,width,height)
{
    gtkMapViewPort = gtk_builder_get_object(GtkUserInterface::builder, "gtkMapViewPort");
    gtkMapFrame = gtk_builder_get_object(GtkUserInterface::builder, "gtkMapFrame");
    scrolledwindowMapUI = gtk_builder_get_object(GtkUserInterface::builder, "scrolledwindowMapUI");

    drawingArea = gtk_drawing_area_new();
    gtk_widget_set_can_focus(drawingArea, true);

    gtk_widget_add_events(drawingArea, GDK_POINTER_MOTION_MASK);
    gtk_widget_add_events(drawingArea, GDK_BUTTON_PRESS_MASK);
    gtk_widget_add_events(drawingArea, GDK_BUTTON_RELEASE_MASK);
    gtk_widget_add_events(drawingArea, GDK_KEY_PRESS_MASK);
    gtk_widget_add_events(drawingArea, GDK_KEY_RELEASE_MASK);
    gtk_widget_add_events(drawingArea, GDK_LEAVE_NOTIFY_MASK);
    gtk_widget_add_events(drawingArea, GDK_ENTER_NOTIFY_MASK);
    gtk_widget_add_events(drawingArea, GDK_ENTER_NOTIFY_MASK);
    

    gtk_widget_set_size_request(GTK_WIDGET(drawingArea), getWidth() * REALMZ_GRID_SIZE + 6 * REALMZ_GRID_SIZE, getHeight() * REALMZ_GRID_SIZE + 6 * REALMZ_GRID_SIZE);
    gtk_container_add(GTK_CONTAINER(gtkMapViewPort), drawingArea);

    g_signal_connect(G_OBJECT(drawingArea), "draw", G_CALLBACK(static_cb_draw_callback), this);
    g_signal_connect(G_OBJECT(drawingArea), "button-press-event", G_CALLBACK(static_cb_clickNotify), this);
    g_signal_connect(G_OBJECT(drawingArea), "button-release-event", G_CALLBACK(static_cb_clickNotify), this);
    g_signal_connect(G_OBJECT(drawingArea), "key-press-event", G_CALLBACK(static_cb_clickNotify), this);
    g_signal_connect(G_OBJECT(drawingArea), "key-release-event", G_CALLBACK(static_cb_clickNotify), this);
    g_signal_connect(G_OBJECT(drawingArea), "motion-notify-event", G_CALLBACK(static_cb_MotionNotify), this);
    g_signal_connect(G_OBJECT(drawingArea), "enter-notify-event", G_CALLBACK(static_cb_onEnter), this);
    g_signal_connect(G_OBJECT(drawingArea), "leave-notify-event", G_CALLBACK(static_cb_onLeave), this);

    g_signal_connect(gtkMapViewPort, "size-allocate", G_CALLBACK(static_my_getsize), this);
    g_signal_connect(G_OBJECT(scrolledwindowMapUI), "scroll-child", G_CALLBACK(static_cb_scroll_child), this);

    thingIsSelected = false;
    ctrlModes = DRAWING_EMPTY;

    cursorPixelbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, true, 8, REALMZ_GRID_SIZE, REALMZ_GRID_SIZE);

    gridColor.setXYZ(0.95f, 0.95f, 0.1f);
    canDrawMouseShadowSquare = false;
    _map_layer = 0;
    updateMapView();

    if (_pixelbuf_unity_grid == nullptr)
	{
		GError* err = NULL;
		_pixelbuf_unity_grid = gdk_pixbuf_new_from_file("ui_imgs//grid_unity.png", &err);
	}
    _grid_enable = true;
}

void ui::MapUI::static_my_getsize(GtkWidget* widget, GtkAllocation* allocation, void* data)
{
    return reinterpret_cast<MapUI*>(data)->map_resize(widget, allocation, allocation);
}

gboolean ui::MapUI::cb_draw_callback(GtkWidget* widget, cairo_t* cr, gpointer data)
{
    // draw grid //
   // if (_grid_enable)
    {
        //cairo_set_source_surface(cr, _surface_grid, 0, 0);
        //cairo_paint(cr);
    }

    //if (_scroll_x_position < 3 * REALMZ_GRID_SIZE)
        ///_scroll_x_position = ;

    cairo_set_source_surface(cr, _surface_grid, 3 * REALMZ_GRID_SIZE + _scroll_x_position, 3 * REALMZ_GRID_SIZE + _scroll_y_position);
    cairo_paint(cr);
    //drawMap(cr, math::Vec2(3, 3), math::Vec2(getWidth(), getHeight()));

    // mouse square - shadow //
    if (canDrawMouseShadowSquare)
    {
        GdkRGBA HLSColor; HLSColor.red = 0; HLSColor.green = 0; HLSColor.blue = 0.85; HLSColor.alpha = 0.25;
        graphics::drawSquare(cr, mousePosition.getX() * REALMZ_GRID_SIZE, mousePosition.getY() * REALMZ_GRID_SIZE, REALMZ_GRID_SIZE, REALMZ_GRID_SIZE, HLSColor);
    }
    
    return FALSE;
}

void ui::MapUI::static_cb_draw_callback(GtkWidget* widget, cairo_t* cr, gpointer data) 
{
    reinterpret_cast<MapUI*>(data)->cb_draw_callback(widget, cr, data);
}

gboolean ui::MapUI::static_cb_clickNotify(GtkWidget* widget, GdkEvent* event, gpointer user_data)
{
    return reinterpret_cast<MapUI*>(user_data)->cb_clickNotify(widget, event, user_data);
}

gboolean ui::MapUI::static_cb_MotionNotify(GtkWidget* widget, GdkEventMotion* event, gpointer user_data)
{
    return reinterpret_cast<MapUI*>(user_data)->cb_MotionNotify(widget, event, user_data);
}

gboolean ui::MapUI::static_cb_onEnter(GtkWidget* widget, GdkEvent* event, gpointer user_data)
{
    return reinterpret_cast<MapUI*>(user_data)->cb_onEnter(widget, event, user_data);
}

gboolean ui::MapUI::static_cb_onLeave(GtkWidget* widget, GdkEvent* event, gpointer user_data)
{
    return reinterpret_cast<MapUI*>(user_data)->cb_onLeave(widget, event, user_data);
}

gboolean ui::MapUI::static_cb_scroll_child(GtkScrolledWindow* scrolled_window,
    GtkScrollType      scroll,
    gboolean           horizontal,
    gpointer           user_data)
{
    return reinterpret_cast<MapUI*>(user_data)->scroll_child(scrolled_window, scroll, horizontal, user_data);
}

gboolean ui::MapUI::cb_MotionNotify(GtkWidget* widget, GdkEventMotion* e, gpointer user_data)
{
    mousePosition.setX((int)(e->x / REALMZ_GRID_SIZE) );
    mousePosition.setY((int)(e->y / REALMZ_GRID_SIZE) );

    if (ctrlModes == DRAWING_PEN_SELECTED &&
        mousePositionPrevious != mousePosition) // we only add new item if mouse square changes //
    {
        mousePositionPrevious = mousePosition;
        // ctrl add operation to the stack //
        ctrlMap->add_ctrlz(ctrl::sOperation(ctrl::eOperation::ADD_THING, addThingMapUI()));
        ctrlMap->add_last_operation(ctrl::eManipulator::OPERATION);
    }

    if (ctrlModes == DRAWING_ERASER_SELECTED &&
      mousePositionPrevious != mousePosition) // we only add new item if mouse square changes //
    {
        mousePositionPrevious = mousePosition;
        delThingMapUI();
        ctrlMap->add_last_operation(ctrl::eManipulator::OPERATION);
    }

    if (ctrlModes == MOVING_VIEW_OF_MAP)
    {
        // mouse distance vector
        mapDetachment.setX((mousePosition.getX() - mouseStartPositionToMoveMapView.getX()) * REALMZ_GRID_SIZE);
        mapDetachment.setY((mousePosition.getY() - mouseStartPositionToMoveMapView.getY()) * REALMZ_GRID_SIZE);
        
        updateMapView();
        mouseStartPositionToMoveMapView = mousePosition; // reset the position
        forceRedraw();
    }

    gtk_widget_queue_draw(GTK_WIDGET(drawingArea));
    return TRUE;
}

void ui::MapUI::updateMapView()
{
    GtkScrolledWindow* scrolledWindow = GTK_SCROLLED_WINDOW(scrolledwindowMapUI);
    GtkAdjustment* h_adjustment = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(scrolledwindowMapUI));
    GtkAdjustment* v_adjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(scrolledwindowMapUI));
    double h_value = gtk_adjustment_get_value(h_adjustment);
    double v_value = gtk_adjustment_get_value(v_adjustment);
    gtk_adjustment_set_value(h_adjustment, h_value + mapDetachment.getX());
    gtk_adjustment_set_value(v_adjustment, v_value + mapDetachment.getY());

    _view_center.setXY(gtk_adjustment_get_value(h_adjustment), gtk_adjustment_get_value(v_adjustment));

    _scroll_x_position = gtk_adjustment_get_value(h_adjustment);
    _scroll_y_position = gtk_adjustment_get_value(v_adjustment);
    std::cout << "?" << gtk_adjustment_get_value(h_adjustment) << "," << gtk_adjustment_get_value(v_adjustment) << std::endl;
    forceRedraw();
}

gboolean ui::MapUI::cb_clickNotify(GtkWidget* widget, GdkEvent* event, gpointer user_data)
{
    if (event->type == GDK_BUTTON_PRESS)
    {
      if (gDrawingToolUI->getDrawingMode() == def::DrawingToolMode::DRAWING_BRUSH)
      {
        // ctrl add operation to the stack //
        ctrlMap->add_ctrlz(ctrl::sOperation(ctrl::eOperation::ADD_THING, addThingMapUI()));
        ctrlMap->add_last_operation(ctrl::eManipulator::OPERATION);

        mousePositionPrevious = mousePosition;
        ctrlModes = DRAWING_PEN_SELECTED;
      }
      else if (gDrawingToolUI->getDrawingMode() == def::DrawingToolMode::DRAWING_ERASE)
      {
        delThingMapUI();
        mousePositionPrevious = mousePosition;
        ctrlModes = DRAWING_ERASER_SELECTED;
      }
      else if(gDrawingToolUI->getDrawingMode() == def::DrawingToolMode::DRAWING_NONE)
      {
        gAuxUI->printMsg("First selects a drawing tool!");
      }
    }
    else if (event->type == GDK_BUTTON_RELEASE)
    {
        ctrlModes = DRAWING_EMPTY;
    }

    if (event->type == GDK_KEY_PRESS)
    {
      if(event->key.keyval == GDK_KEY_space && ctrlModes != MOVING_VIEW_OF_MAP)
      {
        ctrlModesPrevious = ctrlModes;
        ctrlModes = MOVING_VIEW_OF_MAP;
        mouseStartPositionToMoveMapView = mousePosition;
        selectCursor();
      }

      // CTRL + Z //
      if ((event->key.keyval == GDK_KEY_z || event->key.keyval == GDK_KEY_Z) &&
          event->key.state == GDK_CONTROL_MASK)
      {          
          if (!ctrlMap->empty())
          {
              ctrlMap->add_last_operation(ctrl::eManipulator::CTRL_Z);
              ctrl::sOperation operation = ctrlMap->rem_ctrlz();
              do_reverse_operation(operation);
              ctrlMap->add_ctrly(operation.swap_operation()); // add this op into the another stack //
              forceRedraw();
          }
      }

      // CTRL + Y //
      if ((event->key.keyval == GDK_KEY_y || event->key.keyval == GDK_KEY_Y) &&
          event->key.state == GDK_CONTROL_MASK)
      {
          if (!ctrlMap->inv_empty())
          {
              ctrlMap->add_last_operation(ctrl::eManipulator::CTRL_Y);
              ctrl::sOperation operation = ctrlMap->rem_ctrly();
              do_reverse_operation(operation);
              ctrlMap->add_ctrlz(operation.swap_operation()); // add this op into the another stack //
              forceRedraw();
          }
      }
    }
    else if (event->type == GDK_KEY_RELEASE)
    {
      if (event->key.keyval == GDK_KEY_space)
      {
        ctrlModes = ctrlModesPrevious;
        updateMapView();
        selectCursor();
      }
    }

    return TRUE;
}

void ui::MapUI::setDrawThingObj(data::Thing thing)
{
    drawObj = thing;

    thingIsSelected = true;
}

data::Thing ui::MapUI::addThingMapUI()
{
    data::Thing ret;
    // mouse x is col, y is row //
    if (thingIsSelected)
    {
        ret = addThing(drawObj, mousePosition.getY(), mousePosition.getX(), 0);
        gAuxUI->printMsg("Thing " + drawObj.getName() + " added as ["+ drawObj.getType() + "]!");
        forceRedraw();
    }
    else
    {
        gAuxUI->printMsg("You need To select a Thing before drawn!");
    }
    forceRedraw();
    return ret;
}


void ui::MapUI::delThingMapUI()
{
  this->cleansCylinder(mousePosition.getY(), mousePosition.getX(), 0);
  forceRedraw();
}

void ui::MapUI::delThingMapUI(std::string thing_name, math::Vec3<int> thing_position)
{
    this->removeThing(thing_name, thing_position.getY(), thing_position.getX(), thing_position.getZ());
    forceRedraw();
}

void ui::MapUI::delThingMapUI(data::Thing* ptr)
{
    math::Vec3 coords = ptr->getCylinder()->getCoords();
    this->removeThing(ptr->getName(), coords.getY(), coords.getX(), coords.getZ());
    forceRedraw();
}

void ui::MapUI::selectCursor()
{
    // has priority  //
    switch (ctrlModes)
    {
    case MOVING_VIEW_OF_MAP:
        gdk_window_set_cursor(gtk_widget_get_window(GTK_WIDGET(drawingArea)), gdk_cursor_new_for_display(gdk_display_get_default(), GDK_FLEUR));
        return;
    default:
        break;
    }

    switch(gDrawingToolUI->getDrawingMode())
    {
    case def::DrawingToolMode::DRAWING_NONE:
        gdk_window_set_cursor(gtk_widget_get_window(GTK_WIDGET(drawingArea)), gdk_cursor_new_for_display(gdk_display_get_default(), GDK_ARROW));
        break;
    case def::DrawingToolMode::DRAWING_BRUSH:
    {
        if (drawObj.getImgObjPtr() != nullptr)
        {
            math::Vec2<int> textureAtlasReference = drawObj.getImgObjPtr()->getRef(0);
            gdk_pixbuf_copy_area(gResources->getImgPack().getTextureAtlas()->getPixelbuf(), textureAtlasReference.getX() * REALMZ_GRID_SIZE, textureAtlasReference.getY() * REALMZ_GRID_SIZE, REALMZ_GRID_SIZE, REALMZ_GRID_SIZE, cursorPixelbuf, 0, 0);

            gdk_window_set_cursor(gtk_widget_get_window(GTK_WIDGET(drawingArea)), gdk_cursor_new_from_pixbuf(gdk_display_get_default(), cursorPixelbuf, REALMZ_GRID_SIZE / 2, REALMZ_GRID_SIZE / 2));
        }
        else
            gdk_window_set_cursor(gtk_widget_get_window(GTK_WIDGET(drawingArea)), gdk_cursor_new_for_display(gdk_display_get_default(), GDK_PENCIL));
    }
        break;
    case def::DrawingToolMode::DRAWING_ERASE:
        gdk_window_set_cursor(gtk_widget_get_window(GTK_WIDGET(drawingArea)), gdk_cursor_new_for_display(gdk_display_get_default(), GDK_SPRAYCAN));
        break;    
    }
}

gboolean ui::MapUI::cb_onEnter(GtkWidget* widget, GdkEvent* event, gpointer user_data)
{
    // grab focus, we can detect keyboard keys //
    gtk_widget_grab_focus(drawingArea);
    // changes the cursor pixelbuf //
    selectCursor();
    // change grid color //
    gridColor.setXYZ(0.95f, 0.95f, 0.1f);
    canDrawMouseShadowSquare = true;
    return TRUE;
}

gboolean ui::MapUI::cb_onLeave(GtkWidget* widget, GdkEvent* event, gpointer user_data)
{
    // outside mapUI we use the default cursor //
    gdk_window_set_cursor(gtk_widget_get_window(GTK_WIDGET(drawingArea)), gdk_cursor_new_for_display(gdk_display_get_default(), GDK_ARROW));
    // reset ctrls //
    ctrlModes = DRAWING_EMPTY;
    // change grid color //
    gridColor.setXYZ(0.5f,0.5f,0.5f);
    canDrawMouseShadowSquare = false;
    forceRedraw();
    return TRUE;
}

void ui::MapUI::deletAllThingsFromTheMap(std::string thingName)
{
  deletAllThings(thingName);
}

void ui::MapUI::forceRedraw()
{
  gtk_widget_queue_draw(GTK_WIDGET(drawingArea)); // force redraw map //
}

void ui::MapUI::do_reverse_operation(ctrl::sOperation operation)
{
    switch (operation._operation)
    {
    case ctrl::eOperation::ADD_THING:
    {
        delThingMapUI(&operation._thing);
    }
    break;
    case ctrl::eOperation::REMOVE_THING:
    {
        math::Vec3 position = operation._thing.getCylinder()->getCoords();
        addThing(operation._thing, position.getY(), position.getX(), position.getZ());
    }
    break;
    default:
        break;
    }
}

void ui::MapUI::map_resize(GtkWidget* widget, GtkAllocation* allocation, void* data)
{        
    viewWidth = allocation->width;
    viewHeight = allocation->height;

    std::cout << "viewWidth:" << viewWidth << "," << "viewHeight:" << viewHeight << std::endl;
    if (_pixelbuf_full_Grid == NULL)
        _pixelbuf_full_Grid = gdk_pixbuf_new(GDK_COLORSPACE_RGB, true, 8, viewWidth + REALMZ_GRID_SIZE, viewHeight + REALMZ_GRID_SIZE);

    
    if (_pixelbuf_full_Grid != NULL)
    {
        GError* err = NULL;
        for (int l = 0; l < viewHeight; l+= REALMZ_GRID_SIZE)
            for (int c = 0; c < viewWidth; c+= REALMZ_GRID_SIZE)
                gdk_pixbuf_copy_area(_pixelbuf_unity_grid, 0, 0, REALMZ_GRID_SIZE, REALMZ_GRID_SIZE, _pixelbuf_full_Grid, c, l );

        _surface_grid = gdk_cairo_surface_create_from_pixbuf(_pixelbuf_full_Grid, 0, NULL);
        g_object_unref(_pixelbuf_full_Grid);
        _pixelbuf_full_Grid = NULL;
    }
}


gboolean ui::MapUI::scroll_child(GtkScrolledWindow* scrolled_window,
    GtkScrollType      scroll,
    gboolean           horizontal,
    gpointer           user_data)
{
     GtkAdjustment* h_adjustment = gtk_scrolled_window_get_hadjustment(scrolled_window);
     GtkAdjustment* v_adjustment = gtk_scrolled_window_get_vadjustment(scrolled_window);
     double h_value = gtk_adjustment_get_value(h_adjustment);
     double v_value = gtk_adjustment_get_value(v_adjustment);
     gtk_adjustment_set_value(h_adjustment, h_value + mapDetachment.getX());
     gtk_adjustment_set_value(v_adjustment, v_value + mapDetachment.getY());

     _scroll_x_position = gtk_adjustment_get_value(h_adjustment);
     _scroll_y_position = gtk_adjustment_get_value(v_adjustment);
     std::cout <<"?"<< gtk_adjustment_get_value(h_adjustment) << "," << gtk_adjustment_get_value(v_adjustment) << std::endl;
     forceRedraw();
     return TRUE;
}