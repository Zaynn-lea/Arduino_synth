
/*
  Header file modelling a queue ADS using a double linked list
  It has to be doubly linked to optimise the pop function

  Standard queue implementation, with some extra options I needed for the arduino project

  No namespace because of my peculiar nameing scheme

  -------------------------------------------------------------------------------------------------

  made by :
    - Gély Léandre :: https://github.com/Zaynn-lea

  dates :
    - started :      30 / 11 / 2024
    - last updated :  2 / 12 / 2024
*/


#include <assert.h>


#ifndef __QUEUE_HEADER__

#define __QUEUE_HEADER__


/*
  ++--------------++
  ||  Structures  ||
  ++--------------++
*/

template<typename T>
struct zNode
{
  T       tValue;
  zNode * prev;
  zNode * next;
};
template<typename T>
using sNode = struct zNode<T> *;


template<typename T>
struct zQueue
{
  sNode<T> first;
  sNode<T> last;
};
template<typename T>
using Queue = struct zQueue<T>;


/*
  ++--------------------------++
  ||  Functions From the ADT  ||
  ++--------------------------++
*/

template<typename T>
Queue<T> createEmptyQueue(T sample)
{
  /*
    Returns an empty queue of type T

    It need the sample to know which type it's in
  */
  Queue<T> queue;

  queue.first = NULL;
  queue.last  = NULL;

  return queue;
}


template<typename T>
bool isEmptyQueue(Queue<T> queueToTest)
{
  /*
    True if the queue is empty
    False otherwise
  */

  return (queueToTest.first == NULL) && (queueToTest.last == NULL);
}


template<typename T>
void push(T tNewElt, Queue<T> * pQueue)
{
  /*
    Add the new element to the queue 
  */

  // create the new element
  sNode<T> pNewNode = malloc(sizeof(sNode<T>));
  
  pNewNode->tValue = tNewElt;
  pNewNode->next   = pQueue->last;
  pNewNode->prev   = NULL;

  // links it
  pQueue->last = pNewNode;

  if (pQueue->first == NULL)
  {
    pQueue->first = pNewNode;
  }
  else
  {
    // Pointer chain to associate the previous link of prior last node to the newly made node
    pQueue->last->next->prev = pNewNode;
  }
}


template<typename T>
T pop(Queue<T> * pQueue)
{
  /*
    Returns the head of the queue and take it off the queue

    !!  The Queue Can't be empty !!
  */
  assert(! isEmptyQueue(*pQueue));

  sNode<T> temp  = pQueue->first->prev;
  T        tHead = pQueue->first->tValue;

  // getting rid of the head
  free(pQueue->first);
  
  if (temp == NULL)
  {
    pQueue->first = NULL;
    pQueue->last  = NULL;
  }
  else
  {
    pQueue->first = temp;
    temp  ->next  = NULL;
  }

  return tHead;
}


template<typename T>
T head(Queue<T> queue)
{
  /*
    Clearner way to access the first element of the list
  */
  assert( ! isEmptyQueue(queue));

  return queue.first->tValue;
}


/*
  ++----------------------------++
  ||  Extra, Usefull Functions  ||
  ++----------------------------++
*/


template<typename T>
void removeEltQueue(Queue<T> * pQueue, T tElt)
{
  /*
    search an element in the queue and remove it from it 
  */
  sNode<T> temp = pQueue->last;
  
  bool isFound = false;

  if (! isEmptyQueue(*pQueue))
  {
    // Searching our element, if it's in the queue
    while (( ! isFound) && (temp != NULL))
    {
      if (temp->tValue == tElt)
      {
        isFound = true;
      }
      else
      {
        temp = temp->prev;
      }
    }

    // Deleting the element, if it's found
    if (isFound)
    {
      // here temp is the element we want to get rid of

      // stitching together the two end of the list
      temp->prev->next = temp->next;
      temp->next->prev = temp->prev;

      // we don't free the memory, we just want it off the list
    }
  }
}


#endif
