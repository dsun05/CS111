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
  u32 remaining_time;
  u32 waiting_time;
  u32 response_time;
};

TAILQ_HEAD(process_list, process);

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

  int fd = open(path, O_RDONLY);
  if (fd == -1)
  {
    int err = errno;
    perror("open");
    exit(err);
  }

  struct stat st;
  if (fstat(fd, &st) == -1)
  {
    int err = errno;
    perror("stat");
    exit(err);
  }

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

  struct process_list list;
  TAILQ_INIT(&list);

  u32 total_waiting_time = 0;
  u32 total_response_time = 0;


  // pointer to most recently added process (for easy access of the next one)
  struct process *last_added_process = NULL;
  int process_list_size = 0;

  //For each quantum: 
  for(int i = 0; i!=-1; i++){

    int slice_start_time = i*quantum_length;
    int slice_end_time = slice_start_time + quantum_length;
    struct process *head;

    printf("\n");
    printf("==========================================\n");
    printf("DEBUG: QUANTUM %d START (time %d-%d)\n", i, slice_start_time, slice_end_time - 1);

    //Then, we run processes for the length of the quantum
    for(int time = slice_start_time; time < slice_end_time; time++)
    {
      printf("DEBUG: Time %d - Checking for new arrivals\n", time);
      
      //Add processes that arrive during time to the end of the list
      for(int j = last_added_process ? (last_added_process - data + 1) : 0; j < size; j++)
      {
        if(data[j].arrival_time <= time)
        {
          printf("DEBUG: Adding process %d to queue at time %d\n", data[j].pid, time);
          data[j].remaining_time = data[j].burst_time;
          data[j].waiting_time = 0;
          data[j].response_time = 0;
          TAILQ_INSERT_TAIL(&list, &data[j], pointers);
          last_added_process = &data[j];
          process_list_size++;
          printf("DEBUG: Queue size now %d\n", process_list_size);
        }else
        {
          break;
        }
      }

      //Get head process
      if(TAILQ_EMPTY(&list)){
        printf("DEBUG: Queue empty at time %d, breaking\n", time);
        break;
      }else{
        head = TAILQ_FIRST(&list);
        printf("DEBUG: Head process is %d (remaining: %d)\n", head->pid, head->remaining_time);
      }

      //Set response time
      if(head->response_time == 0)
      {
        if(head->remaining_time == head->burst_time)
        {
          head->response_time = time - head->arrival_time;
          printf("DEBUG: Set response time for process %d to %d\n", head->pid, head->response_time);
        }
      }

      //Decrement head remaining time
      printf("DEBUG: Decrementing remaining time for process %d from %d to %d\n", head->pid, head->remaining_time, head->remaining_time - 1);
      head->remaining_time--;

      //If head's remaining time is filled, remove from list. 
      if(head->remaining_time < 1)
      {
        printf("DEBUG: Process %d completed, removing from queue\n", head->pid);
        process_list_size--;
        total_waiting_time += head->waiting_time;
        total_response_time += head->response_time;
        printf("DEBUG: Process %d final waiting time: %d, response time: %d\n", head->pid, head->waiting_time, head->response_time);
        TAILQ_REMOVE(&list, head, pointers);
        printf("DEBUG: Queue size after removal: %d\n", process_list_size);
        if(TAILQ_EMPTY(&list)){
          printf("DEBUG: Queue empty after removal, breaking\n");
          break;
        }
      }

      //Increment all other processes' waiting and response times
      printf("DEBUG: Incrementing waiting time for processes after head\n");
      struct process *curr = TAILQ_NEXT(head, pointers);
      while(curr != NULL) 
      {
          printf("DEBUG: Incrementing waiting time for process %d from %d to %d\n", curr->pid, curr->waiting_time, curr->waiting_time + 1);
          curr->waiting_time++;
          curr = TAILQ_NEXT(curr, pointers);
      }
    }
    
    //After each time quantum, we move the current head to the back, and replace the head with the next element. 
    printf("DEBUG: Moving head process %d to back of queue\n", head->pid);
    TAILQ_REMOVE(&list, head, pointers);
    TAILQ_INSERT_TAIL(&list, head, pointers);

    printf("DEBUG: QUANTUM %d END\n", i);
    printf("==========================================\n");

    //If the list is empty after this quantum, exit. 
    if(TAILQ_EMPTY(&list)){
      printf("DEBUG: Queue empty after quantum, exiting\n");
      break;
    }
  }

  printf("Average waiting time: %.2f\n", (float)total_waiting_time / (float)size);
  printf("Average response time: %.2f\n", (float)total_response_time / (float)size);
  free(data);
  return 0;
}
