/*
 * timerExample.c
 *
 *  Created on: Jul 19, 2019
 *  Author: Prasad.T.V.R
 *
 */
/* Program depicting usage of multiple GMainContexts and mainloops 
   to handle things very asynchronously
 */
#include <glib.h>
#include <glib/gstdio.h>
#include <gio/gio.h>

/*Default Mainloop for the main function and the application*/
static GMainLoop *loop = NULL; 
/*Mainloop for the worker thread*/
static GMainLoop *workLoop = NULL;
/*Timer 1 callback executed on the default mainloop/ context*/
gboolean timerOneFunc (gpointer user_data)
{
	g_print("Timer 1 is ticking\n");
	return TRUE;
}
/*Timer 2 callback executed on the worker loop and the worker context*/
gboolean timerTwoFunc (gpointer user_data)
{
	g_print("Timer 2 is ticking\n");
	return TRUE;
}
/*The worker thread*/
gpointer workerThreadFunc (gpointer data)
{
	GMainContext *workerContext;
	/*Create a new GMainContext and mainloop for the worker thread
	  so that it does not depend on the default application GMainCotext and mainloop
	 */
	workerContext = g_main_context_new();
	workLoop = g_main_loop_new(workerContext,FALSE);
	GSource *timerSource = g_timeout_source_new_seconds(5);
	g_source_set_callback (timerSource, timerTwoFunc, workLoop, NULL);
	guint timerTwo = g_source_attach(timerSource,workerContext);
	/* Get the worker thread mainloop going */
	g_main_loop_run(workLoop);

	g_main_context_unref(workerContext);
	g_source_unref(timerSource);

}

int main (void)
{

	GThread *workerThread;

	loop = g_main_loop_new(NULL,FALSE);

	guint timerOne = g_timeout_add_seconds(3,timerOneFunc, NULL);

	workerThread = g_thread_new("WorkerThread",workerThreadFunc,NULL);

	g_main_loop_run(loop);
	g_thread_join(workerThread);

	return 0;
}

