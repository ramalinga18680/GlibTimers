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
static void g_task_callback(GObject *object,GAsyncResult *result,gpointer user_data)
{
	GError *error = NULL;
	g_print("In g_task_callback\n");

	if(g_task_propagate_boolean(G_TASK(result),&error)){
		g_print("g_task is successful\n");

	}else{
		g_print("g_task failed\n");
	}

}
static void g_task_thread(GTask *task,gpointer source_object,gpointer task_data,GCancellable *cancellable)
{
	GError *local_error=NULL;
	g_print("I am in g_task_thread\n");
	g_set_error(&local_error,G_OPTION_ERROR,G_OPTION_ERROR_FAILED,"the g_task failed");
	//g_task_return_boolean(task,TRUE);
	g_task_return_error(task,local_error);
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

	GTask * task = g_task_new(NULL,NULL,g_task_callback,NULL);
	g_task_set_task_data(task,NULL,NULL);
	g_task_run_in_thread(task,g_task_thread);
	g_main_loop_run(loop);
	g_thread_join(workerThread);

	return 0;
}

