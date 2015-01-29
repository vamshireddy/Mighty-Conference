#ifndef WEBSERVER_H
#define WEBSERVER_H
#define WS_RUNNING 1
#define WS_STOP 0
#include <Python.h>
#include<unistd.h>

int ws_state;
extern int sock_fd;
PyObject * pModule; //object to hold module


/* Function to send data */

static PyObject* pyio_test(PyObject *self, PyObject *args){
	return Py_BuildValue("s", "someday");
}

static PyObject* pyio_write(PyObject *self, PyObject *args)
{
	//parse arguments
	const char * data;
	int len;
 
	if (!PyArg_ParseTuple(args, "si", &data,&len))
           return NULL;
	printf("this:%s , %d \n",data,len);
	//will take string and length

	if( write(sock_fd,data,len) < len)
	{
		perror("Error in writing\n");
		
	}
	return NULL;
}

/* Function to get data */

static PyObject* pyio_read(PyObject *self, PyObject *args)
{
	//parse arguments
	int len;
 
	if (!PyArg_ParseTuple(args, "i", &len))
           return NULL;	
	
	//will take the number of characters to be read from the socket
	char buffer[len];
	int char_count = len;
	int chars_read = 0;

	while( ( chars_read = read(sock_fd, buffer + chars_read , char_count ) ) > 0 )
	{
		char_count = char_count - chars_read;
		if( char_count == 0 )
		{
			// All chars are read, break out
			break;
		}
	}

	if( chars_read == -1 )
	{
		perror("Error in reading the line in readLine function : handle_client.h\n");
		return NULL;
	}
	else if( chars_read == 0 )
	{
		printf("Client's connection is terminated\n");
		return NULL;
	}

	return Py_BuildValue("s", buffer);
}

//all functions in the extended module
static PyMethodDef pyio_methods[] = {
    {"read", pyio_read, METH_VARARGS,
     "to send data to the server"},
    {"write", pyio_write, METH_VARARGS,
     "to send data to the server"},
    {"test", pyio_test, METH_VARARGS,
     "to test the module"},
    {NULL, NULL, 0, NULL}
};


//need to create a new thread for the server
void* ws_start(void * args){
	//args is null 
      PyObject 
   *pName, //object to hold script name string
   *pFunc; //object to hold function


    Py_Initialize(); //init the interpreter
   //extend the embedded interpreter for sending data to t
 
   //extension module functions added to the embedded intepreter	
   Py_InitModule("pyio",  pyio_methods);
   //functions added to interpreter

    PySys_SetPath("./../Webserver:/usr/lib/python2.7"); //all the webserver files will be in this
    pName = PyString_FromString("ws");
    //created a python string	
    /* Error checking of pName left out */

    pModule = PyImport_Import(pName);
	//pModule now importing the webserver module
    Py_DECREF(pName);    //script name object not needed so reference being decremented
	
    if (pModule != NULL) {
        pFunc = PyObject_GetAttrString(pModule, "main");
        /* pFunc is a new reference */

        if (pFunc && PyCallable_Check(pFunc)) {
		//the main function in module ws is present and is callable	
            PyObject_CallObject(pFunc, NULL);
	    ws_state=WS_RUNNING;
        }
        else {
            if (PyErr_Occurred())
                PyErr_Print();
            fprintf(stderr, "Cannot find function \"%s\"\n", "mains");
        }
        Py_XDECREF(pFunc);
        Py_DECREF(pModule);
    }
    else {
        PyErr_Print();
        fprintf(stderr, "Failed to load \"%s\"\n", "webserver.py");
        return ;
    }
	//need  to keep the thread alive
	while(ws_state==WS_RUNNING){}
	Py_Finalize();
}

//function to embed the py interpreter and start the webserver
int ws_init(){
    pthread_t tid;
    if( pthread_create(&tid, NULL, ws_start, NULL) != 0)
	{
		printf("Failed to spawn a thread for the python webserver");
		return -1;
	}
}

void ws_finalize(){
	//to terminate the thread
	ws_state= WS_STOP;
}
#endif
