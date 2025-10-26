#include <stdio.h>
#include <setjmp.h>
#include <stdlib.h>
#include <string.h>

// 任務狀態枚舉
typedef enum {
    TASK_READY,     // 就緒
    TASK_RUNNING,   // 執行中
    TASK_BLOCKED,   // 阻塞
    TASK_FINISHED   // 完成
} TaskState;

// 任務控制塊
typedef struct {
    jmp_buf context;
    int priority;       // 數字越大優先權越高
    TaskState state;
    const char *name;
    int work_count;     // 工作計數器
    int max_work;       // 最大工作次數
    int initialized;    // 標記是否已初始化
} Task;

#define MAX_TASKS 3
Task tasks[MAX_TASKS];
Task *current_task = NULL;
int task_count = 0;
jmp_buf scheduler_context;  // 排程器的上下文

// 找到最高優先權的就緒任務
Task* find_highest_priority_task() {
    Task *highest = NULL;
    for (int i = 0; i < task_count; i++) {
        if (tasks[i].state == TASK_READY) {
            if (highest == NULL || tasks[i].priority > highest->priority) {
                highest = &tasks[i];
            }
        }
    }
    return highest;
}

// 任務讓出 CPU（協作式調度）
void task_yield() {
    if (current_task == NULL) return;
    
    Task *prev_task = current_task;
    Task *next_task = find_highest_priority_task();
    
    if (next_task == NULL) {
        // 沒有其他就緒任務
        return;
    }
    
    if (next_task == current_task) {
        // 只有當前任務在執行
        return;
    }
    
    // 保存當前任務上下文
    int val = setjmp(prev_task->context);
    if (val == 0) {
        // 第一次返回：保存上下文並切換
        printf("[SCHEDULER] Context switch: %s (P%d) -> %s (P%d)\n",
               prev_task->name, prev_task->priority,
               next_task->name, next_task->priority);
        
        prev_task->state = TASK_READY;
        current_task = next_task;
        next_task->state = TASK_RUNNING;
        
        // 確保在有效的上下文中切換
        if (next_task->initialized) {
            longjmp(next_task->context, 1);
            // 永遠不會到達這裡
        } else {
            printf("[ERROR] Attempting to switch to uninitialized task %s\n", 
                   next_task->name);
            current_task = prev_task;
            prev_task->state = TASK_RUNNING;
        }
    } else {
        // 恢復後繼續執行 (val == 1)
        return;
    }
}

// 模擬中斷服務程式
void simulate_interrupt(int interrupt_id) {
    printf("\n[ISR] Interrupt %d occurred!\n", interrupt_id);
    
    // 可以在這裡喚醒特定任務
    for (int i = 0; i < task_count; i++) {
        if (tasks[i].state == TASK_BLOCKED) {
            tasks[i].state = TASK_READY;
            printf("[ISR] Task %s unblocked\n", tasks[i].name);
        }
    }
    
    // 不立即觸發調度，讓當前任務繼續執行到下一次 yield
    // task_yield();  // 移除這行
}

// 任務完成 - 傳入要完成的任務
void task_finish(Task *finished_task) {
    if (finished_task) {
        finished_task->state = TASK_FINISHED;
        printf("[TASK] %s finished\n", finished_task->name);
        
        // 返回到scheduler，不嘗試切換到其他任務
        printf("[SCHEDULER] Task finished, returning to scheduler\n");
        current_task = NULL;
        longjmp(scheduler_context, 1);
    }
}

// 任務函式定義
void task1_func() {
    // 初始化任務上下文
    if (setjmp(tasks[0].context) == 0) {
        tasks[0].initialized = 1;
        return; // 初始化完成，返回
    }
    
    // 從這裡開始執行任務
    printf("[TASK1] Started (Priority: %d)\n", tasks[0].priority);
    
    while (tasks[0].work_count < tasks[0].max_work) {
        printf("[TASK1] Working... (%d/%d)\n", 
               tasks[0].work_count + 1, tasks[0].max_work);
        
        // 模擬工作負載
        for (volatile int i = 0; i < 50000000; i++);
        
        tasks[0].work_count++;
        
        // 主動讓出 CPU
        task_yield();
    }
    
    // 先設置為FINISHED狀態
    tasks[0].state = TASK_FINISHED;
    task_finish(&tasks[0]);
}

void task2_func() {
    if (setjmp(tasks[1].context) == 0) {
        tasks[1].initialized = 1;
        return;
    }
    
    printf("[TASK2] Started (Priority: %d)\n", tasks[1].priority);
    
    while (tasks[1].work_count < tasks[1].max_work) {
        printf("[TASK2] Working... (%d/%d)\n", 
               tasks[1].work_count + 1, tasks[1].max_work);
        
        for (volatile int i = 0; i < 50000000; i++);
        
        tasks[1].work_count++;
        
        // 只在第一次阻塞
        if (tasks[1].work_count == 1) {
            printf("[TASK2] Blocking itself...\n");
            tasks[1].state = TASK_BLOCKED;
            task_yield();
            printf("[TASK2] Resumed after unblock\n");
            fflush(stdout);
        } else {
            task_yield();
        }
    }
    
    tasks[1].state = TASK_FINISHED;
    task_finish(&tasks[1]);
}

void task3_func() {
    if (setjmp(tasks[2].context) == 0) {
        tasks[2].initialized = 1;
        return;
    }
    
    printf("[TASK3] Started (Priority: %d)\n", tasks[2].priority);
    
    while (tasks[2].work_count < tasks[2].max_work) {
        printf("[TASK3] Working... (%d/%d)\n", 
               tasks[2].work_count + 1, tasks[2].max_work);
        
        for (volatile int i = 0; i < 50000000; i++);
        
        tasks[2].work_count++;
        
        // 模擬觸發中斷
        if (tasks[2].work_count == 2) {
            simulate_interrupt(1);
        }
        
        task_yield();
    }
    
    printf("[TASK3] Loop finished, calling task_finish\n");
    // 先設置為FINISHED狀態，防止被選中
    tasks[2].state = TASK_FINISHED;
    task_finish(&tasks[2]);
}

// 初始化任務
void init_task(int id, const char *name, int priority, int max_work, void (*func)()) {
    memset(&tasks[id], 0, sizeof(Task));  // 清空任務結構
    tasks[id].priority = priority;
    tasks[id].state = TASK_READY;
    tasks[id].name = name;
    tasks[id].work_count = 0;
    tasks[id].max_work = max_work;
    tasks[id].initialized = 0;
    
    // 初始化任務上下文
    func();
    
    task_count++;
}

// 啟動排程器
void start_scheduler() {
    printf("\n=== Scheduler Started ===\n\n");
    
    // 持續運行，直到沒有任務
    while (1) {
        // 設置返回點
        if (setjmp(scheduler_context) != 0) {
            // 從任務返回，繼續調度下一個任務
            printf("[SCHEDULER] Task completed, looking for next task...\n");
        }
        
        current_task = find_highest_priority_task();
        if (current_task) {
            printf("[SCHEDULER] Found task: %s (P%d), state=%d\n", 
                   current_task->name, current_task->priority, current_task->state);
            
            // 確保任務是READY狀態
            if (current_task->state != TASK_READY) {
                printf("[SCHEDULER] Task %s is not READY (state=%d), skipping\n",
                       current_task->name, current_task->state);
                break;
            }
            
            current_task->state = TASK_RUNNING;
            printf("[SCHEDULER] Starting with %s (P%d)\n", 
                   current_task->name, current_task->priority);
            
            if (current_task->initialized) {
                longjmp(current_task->context, 1);
            } else {
                printf("[SCHEDULER] Task not initialized, skipping\n");
                break;
            }
        } else {
            printf("[SCHEDULER] No more tasks to run!\n");
            break;
        }
    }
}

int main() {
    printf("=== Cooperative Task Scheduler Demo ===\n");
    printf("Priority: Higher number = Higher priority\n\n");
    
    // 初始化任務（優先權：Task2 > Task3 > Task1）
    init_task(0, "Task1", 1, 3, task1_func);  // 低優先權
    init_task(1, "Task2", 3, 3, task2_func);  // 高優先權
    init_task(2, "Task3", 2, 3, task3_func);  // 中優先權
    
    // 啟動排程器
    start_scheduler();
    
    printf("\n=== All Tasks Completed ===\n");
    
    return 0;
}
