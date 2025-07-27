#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/queue.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

typedef uint32_t u32;
typedef int32_t i32;

struct process
{
  u32 pid;
  u32 arrival_time;
  u32 burst_time;

  TAILQ_ENTRY(process) pointers;

  /* Additional fields here */
  u32 remaining_time;
  u32 waiting_time;
  int response_time;
  /* End of "Additional fields here" */
};

TAILQ_HEAD(process_list, process);

//Parsing a string representation of a number and converting it to an actual integer value. "123" -> 123. Loose implementation. Ignores leading non-integer characters. Returns current value 
// if non integer is found. Takes file input.
u32 next_int(const char **data, const char *data_end)
{
  u32 current = 0;
  bool started = false;
  while (*data != data_end)
  {
    char c = **data;

    if (c < 0x30 || c > 0x39)
    {
      if (started)
      {
        return current;
      }
    }
    else
    {
      if (!started)
      {
        current = (c - 0x30);
        started = true;
      }
      else
      {
        current *= 10;
        current += (c - 0x30);
      }
    }

    ++(*data);
  }

  printf("Reached end of file while looking for another integer\n");
  exit(EINVAL);
}

//Parsing a string representation of a number and converting it to an actual integer value. "123" -> 123. Strict implementation. Crashes if any non integer character is found. Takes C string input.
u32 next_int_from_c_str(const char *data)
{
  char c;
  u32 i = 0;
  u32 current = 0;
  bool started = false;
  while ((c = data[i++]))
  {
    if (c < 0x30 || c > 0x39)
    {
      exit(EINVAL);
    }
    if (!started)
    {
      current = (c - 0x30);
      started = true;
    }
    else
    {
      current *= 10;
      current += (c - 0x30);
    }
  }
  return current;
}

void init_processes(const char *path,
                    struct process **process_data,
                    u32 *process_size)
{

  //Opening the file
  int fd = open(path, O_RDONLY);
  if (fd == -1)
  {
    int err = errno;
    perror("open");
    exit(err);
  }

  //Getting the file info
  struct stat st;
  if (fstat(fd, &st) == -1)
  {
    int err = errno;
    perror("stat");
    exit(err);
  }

  //Mapping the file to memory. File sized chunk, that is read only, and cannot be accessed by other processes. Aligned to page size address.
  u32 size = st.st_size;
  const char *data_start = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (data_start == MAP_FAILED)
  {
    int err = errno;
    perror("mmap");
    exit(err);
  }

  const char *data_end = data_start + size;
  const char *data = data_start;

  *process_size = next_int(&data, data_end);

  *process_data = calloc(sizeof(struct process), *process_size);
  if (*process_data == NULL)
  {
    int err = errno;
    perror("calloc");
    exit(err);
  }

  for (u32 i = 0; i < *process_size; ++i)
  {
    (*process_data)[i].pid = next_int(&data, data_end);
    (*process_data)[i].arrival_time = next_int(&data, data_end);
    (*process_data)[i].burst_time = next_int(&data, data_end);
  }

  munmap((void *)data, size);
  close(fd);
}

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    return EINVAL;
  }
  struct process *data;
  u32 size;
  init_processes(argv[1], &data, &size);

  u32 quantum_length = next_int_from_c_str(argv[2]);

  //Empty/null list for processes
  struct process_list list;
  TAILQ_INIT(&list);

  u32 total_waiting_time = 0;
  u32 total_response_time = 0;


  // pointer to most recently added process (for easy access of the next one)
  struct process *last_added_process = NULL;
  int process_list_size = 0;

  //Infite loop, this will determine our absoloute sense of "time". i = # of quantums processed, starting at 0
  for(int i = 0; i!=-1; i++){

    //First, we need to loop through the input data and insert all data that has newly arrived at this time
    for(int j = last_added_process ? (last_added_process - data + 1) : 0; j < size; j++){
      if(data[j].arrival_time <= i * quantum_length)
      {
        //add process to list
        data[j].remaining_time = data[j].burst_time;
        data[j].waiting_time = 0;
        data[j].response_time = -1;
        TAILQ_INSERT_TAIL(&list, &data[j], pointers);
        //lastAdded = process
        last_added_process = &data[j];
        process_list_size++;
      }else{
        break;
      }
    }

    printf("Debug: Iteration i = %d, Current time = %d\n", i, i * quantum_length);

    printf("Debug: Queue contents before quantum slice: ");
    if(TAILQ_EMPTY(&list)) {
      printf("EMPTY");
    } else {
      struct process *curr = TAILQ_FIRST(&list);
      while(curr != NULL) {
        printf("P%d(r:%d,w:%d,rt:%d) ", curr->pid, curr->remaining_time, curr->waiting_time, curr->response_time);
        curr = TAILQ_NEXT(curr, pointers);
      }
    }
    printf("\n");

    int finished = 0;
    int requeued = 0;
    struct process *head;
    //Then, we run processes for one time quantum. q = how much of the quantum we have used up so far. 
    for(int q = 0; q<quantum_length;){

      //Get head if list not empty
      if(TAILQ_EMPTY(&list)){
        break;
      }else{
        head = TAILQ_FIRST(&list);
      }

      //if this is the first time head is being processed, calculate and add its response time
      if(head->response_time < 0)
      {
        head->response_time = (i*quantum_length + q)  - (head->arrival_time); 
      } 

      //If the head process will finish within the quantum, we finish the process and start the next one
      if(head->remaining_time <= quantum_length - q)
      {
        finished++;
        q += head->remaining_time;
        total_waiting_time += head->waiting_time;
        // Remove the finished process and get the new head
        TAILQ_REMOVE(&list, head, pointers);
        process_list_size--;


        if(TAILQ_EMPTY(&list)){
          break;
        }else{
          head = TAILQ_FIRST(&list);
        }
      }

      //If the head process will not finish, there are two cases:
      // it is the first process -> decrement remaining time by time quantum length, then requeue and break the loop
      // it is not the first process -> decrement remaining time by quantum length - how much of the quantum we've used so far, then requeue and break the loop
      else
      {
        if(finished > 0){
          head->remaining_time -= (quantum_length - q);

          //how long the head waited before running this time quantum
          head->waiting_time += q;
        }else
        {
          head->remaining_time -= quantum_length;
        }
        // Move head process to back of queue
        TAILQ_REMOVE(&list, head, pointers);
        TAILQ_INSERT_TAIL(&list, head, pointers);
        head = TAILQ_FIRST(&list);
        requeued++;
        break;
      }


      struct process *curr = head;
      for(int w = 0; w <(process_list_size - requeued); w++)
      {
        //waiting time:
        // if waiting time == 0; then this is its first time waiting. waiting time = current time - arrival time
        // else, add time quantume to waiting time

        if(curr->waiting_time == 0)
        {
          curr->waiting_time = ((i+1)*quantum_length) - curr->arrival_time;
        }
        else
        {
          curr->waiting_time += quantum_length;
        }
        curr = TAILQ_NEXT(curr, pointers);
      }

      printf("Debug: Queue contents after quantum slice: ");
      if(TAILQ_EMPTY(&list)) {
        printf("EMPTY");
      } else {
        curr = TAILQ_FIRST(&list);
        while(curr != NULL) {
          printf("P%d(r:%d,w:%d,rt:%d) ", curr->pid, curr->remaining_time, curr->waiting_time, curr->response_time);
          curr = TAILQ_NEXT(curr, pointers);
        }
      }
      printf("-----------------------------------------------\n");
    }

    
    //After each quantum, we check if we have exhausted the processes.
    if(TAILQ_EMPTY(&list))
    {
      break;
    }
  }


  printf("Average waiting time: %.2f\n", (float)total_waiting_time / (float)size);
  printf("Average response time: %.2f\n", (float)total_response_time / (float)size);

  free(data);
  return 0;
}
