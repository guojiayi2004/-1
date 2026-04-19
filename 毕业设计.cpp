#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
// ==================== 数据结构定义 ====================

typedef struct {
    int id;
    char name[20];
    char dept[30];
    int max_num;
    int current_num;
} Doctor;

typedef struct {
    int id;
    char patientName[20];
    int doctorId;
    char doctorName[20];
    char dept[30];
    int sequenceNum;
    char appointmentTime[20];
    int status;
} Appointment;

// 全局变量
Doctor hospitalData[100];
Appointment records[1000];
int docCount = 0;
int recordCount = 0;
int nextRecordId = 1;
const char* DB_FILE = "hospital_data.txt";
const char* RECORD_FILE = "appointment_records.txt";
const char* ADMIN_PWD = "123456";

// ==================== 函数原型 ====================

void loadData();
void saveData();
void loadRecords();
void saveRecords();
void displayDoctors();
void displayDoctorsByDept();
void makeAppointment();
void queryMyAppointment();
void cancelAppointment();
void adminMenu();
void addDoctor();
void deleteDoctor();
void resetDailyQuota();
void clearInputBuffer();
void getCurrentTime(char* buffer);

// ==================== 主程序 ====================
 
int main() {
    // 启动选项
    printf("╔════════════════════════════════╗\n");
    printf("║   医院预约系统  - 启动选项  \n");
    printf("╠════════════════════════════════╣\n");
    printf("║  1. 正常启动                   ║\n");
    printf("║  2. 重置所有数据（解决故障）   ║\n");
    printf("╚════════════════════════════════╝\n");
    printf("请选择: ");
    
    int startupChoice;
    if (scanf("%d", &startupChoice) != 1) {
        clearInputBuffer();
        startupChoice = 1;
    }
    clearInputBuffer();
    
    if (startupChoice == 2) {
        printf("\n警告：即将删除所有数据文件！\n");
        printf("确认重置？(y/n): ");
        char confirm;
        scanf(" %c", &confirm);
        if (confirm == 'y' || confirm == 'Y') {
            remove(DB_FILE);
            remove(RECORD_FILE);
            printf("数据已重置，将使用默认配置启动。\n\n");
        }
    }
    
    // 加载数据
    loadData();
    loadRecords();
    
    int choice;
    while (1) {
        printf("\n╔══════════════════════════════════════════════════╗\n");
        printf("║                    医院预约系统    \n");
        printf("╠══════════════════════════════════════════════════╣\n");
        printf("║  【患者服务】                                     ║\n");
        printf("║    1. 查看所有医生排班                           ║\n");
        printf("║    2. 按科室筛选医生                             ║\n");
        printf("║    3. 办理预约挂号                               ║\n");
        printf("║    4. 查询我的预约                               ║\n");
        printf("║    5. 取消预约                                   ║\n");
        printf("║                                                  ║\n");
        printf("║  【系统功能】                                     ║\n");
        printf("║    6. 管理员入口                                 ║\n");
        printf("║    0. 退出并保存                                 ║\n");
        printf("╚══════════════════════════════════════════════════╝\n");
        printf("请选择操作: ");

        if (scanf("%d", &choice) != 1) {
            printf("错误：请输入有效的数字指令！\n");
            clearInputBuffer();
            continue;
        }

        switch (choice) {
            case 1: displayDoctors(); break;
            case 2: displayDoctorsByDept(); break;
            case 3: makeAppointment(); break;
            case 4: queryMyAppointment(); break;
            case 5: cancelAppointment(); break;
            case 6: adminMenu(); break;
            case 0:
                saveData();
                saveRecords();
                printf("\n系统提示：所有数据已安全保存。\n");
                printf("感谢使用，再见！\n");
                return 0;
            default:
                printf("系统提示：无效指令，请重新输入。\n");
        }
    }
    return 0;
}

// ==================== 增强版数据加载函数 ====================

void loadData() {
    FILE *fp = fopen(DB_FILE, "r");
    if (fp == NULL) {
        printf("提示：医生数据文件不存在，初始化默认数据。\n");
        docCount = 4;
        hospitalData[0] = (Doctor){101, "张仲景", "中医内科", 20, 5};
        hospitalData[1] = (Doctor){102, "华佗",   "外科手术", 15, 14};
        hospitalData[2] = (Doctor){103, "孙思邈", "全科门诊", 30, 0};
        hospitalData[3] = (Doctor){104, "李时珍", "药剂科",   10, 10};
        return;
    }
    
    printf("正在加载医生数据...\n");
    docCount = 0;
    int lineNum = 0;
    char buffer[128];
    
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        lineNum++;
        
        Doctor temp;
        int result = sscanf(buffer, "%d %19s %29s %d %d",
               &temp.id,
               temp.name,
               temp.dept,
               &temp.max_num,
               &temp.current_num);
        
        if (result != 5) {
            printf("  警告：第%d行格式错误，已跳过。\n", lineNum);
            continue;
        }
        
        if (docCount >= 100) {
            printf("  警告：医生数量已达上限(100人)。\n");
            break;
        }
        
        hospitalData[docCount] = temp;
        docCount++;
    }
    
    fclose(fp);
    printf("成功加载%d位医生信息。\n", docCount);
    
    if (docCount == 0) {
        printf("医生数据为空或损坏，使用默认数据。\n");
        docCount = 4;
        hospitalData[0] = (Doctor){101, "张仲景", "中医内科", 20, 5};
        hospitalData[1] = (Doctor){102, "华佗",   "外科手术", 15, 14};
        hospitalData[2] = (Doctor){103, "孙思邈", "全科门诊", 30, 0};
        hospitalData[3] = (Doctor){104, "李时珍", "药剂科",   10, 10};
    }
}

void loadRecords() {
    FILE *fp = fopen(RECORD_FILE, "r");
    if (fp == NULL) {
        printf("提示：预约记录文件不存在，将创建新文件。\n");
        recordCount = 0;
        nextRecordId = 1;
        return;
    }
    
    printf("正在加载预约记录...\n");
    recordCount = 0;
    nextRecordId = 1;
    int lineNum = 0;
    char buffer[256];
    
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        lineNum++;
        
        Appointment temp;
        int result = sscanf(buffer, "%d %19s %d %19s %29s %d %19s %d",
               &temp.id,
               temp.patientName,
               &temp.doctorId,
               temp.doctorName,
               temp.dept,
               &temp.sequenceNum,
               temp.appointmentTime,
               &temp.status);
        
        if (result != 8) {
            printf("  警告：第%d行格式错误（读到%d个字段），已跳过。\n", lineNum, result);
            continue;
        }
        
        if (recordCount >= 1000) {
            printf("  警告：预约记录已达上限(1000条)。\n");
            break;
        }
        
        if (temp.id <= 0 || temp.id > 100000) {
            printf("  警告：第%d行ID异常(%d)，已跳过。\n", lineNum, temp.id);
            continue;
        }
        
        records[recordCount] = temp;
        recordCount++;
        
        if (temp.id >= nextRecordId) {
            nextRecordId = temp.id + 1;
        }
    }
    
    fclose(fp);
    printf("成功加载%d条预约记录，下一个编号为%d。\n", recordCount, nextRecordId);
}

void saveData() {
    FILE *fp = fopen(DB_FILE, "w");
    if (fp == NULL) {
        printf("错误：无法写入医生数据文件！\n");
        return;
    }
    for (int i = 0; i < docCount; i++) {
        fprintf(fp, "%d %s %s %d %d\n", 
                hospitalData[i].id, 
                hospitalData[i].name, 
                hospitalData[i].dept, 
                hospitalData[i].max_num, 
                hospitalData[i].current_num);
    }
    fclose(fp);
}

void saveRecords() {
    FILE *fp = fopen(RECORD_FILE, "w");
    if (fp == NULL) {
        printf("错误：无法写入预约记录文件！\n");
        return;
    }
    for (int i = 0; i < recordCount; i++) {
        fprintf(fp, "%d %s %d %s %s %d %s %d\n",
                records[i].id,
                records[i].patientName,
                records[i].doctorId,
                records[i].doctorName,
                records[i].dept,
                records[i].sequenceNum,
                records[i].appointmentTime,
                records[i].status);
    }
    fclose(fp);
}

// ==================== 患者功能 ====================

void displayDoctors() {
    if (docCount == 0) {
        printf("\n暂无医生信息。\n");
        return;
    }
    
    printf("\n┌─────┬────────────┬────────────────┬──────────┬────────────┐\n");
    printf("│ ID  │ 医生姓名   │ 所属科室       │ 剩余号源 │ 状态       │\n");
    printf("├─────┼────────────┼────────────────┼──────────┼────────────┤\n");
    
    for (int i = 0; i < docCount; i++) {
        int remaining = hospitalData[i].max_num - hospitalData[i].current_num;
        char status[15];
        
        if (remaining <= 0) strcpy(status, "已满号");
        else if (remaining <= 3) strcpy(status, "紧张");
        else strcpy(status, "充足");
        
        printf("│ %-3d │ %-10s │ %-14s │ %2d/%-2d   │ %-10s │\n",
               hospitalData[i].id, hospitalData[i].name, hospitalData[i].dept,
               remaining, hospitalData[i].max_num, status);
    }
    printf("└─────┴────────────┴────────────────┴──────────┴────────────┘\n");
}

void displayDoctorsByDept() {
    char targetDept[30];
    printf("\n请输入要查询的科室名称（如：中医内科）: ");
    scanf("%s", targetDept);
    clearInputBuffer();
    
    int found = 0;
    printf("\n【%s】科室的医生列表：\n", targetDept);
    printf("┌─────┬────────────┬──────────┬────────────┐\n");
    printf("│ ID  │ 医生姓名   │ 剩余号源 │ 状态       │\n");
    printf("├─────┼────────────┼──────────┼────────────┤\n");
    
    for (int i = 0; i < docCount; i++) {
        if (strcmp(hospitalData[i].dept, targetDept) == 0) {
            found = 1;
            int remaining = hospitalData[i].max_num - hospitalData[i].current_num;
            char status[15];
            strcpy(status, remaining > 0 ? "可预约" : "已满");
            
            printf("│ %-3d │ %-10s │ %2d/%-2d   │ %-10s │\n",
                   hospitalData[i].id, hospitalData[i].name,
                   remaining, hospitalData[i].max_num, status);
        }
    }
    printf("└─────┴────────────┴──────────┴────────────┘\n");
    
    if (!found) {
        printf("未找到该科室的医生。\n现有科室：");
        for (int i = 0; i < docCount; i++) {
            printf("%s", hospitalData[i].dept);
            if (i < docCount - 1) printf("、");
        }
        printf("\n");
    }
}

void makeAppointment() {
    char patientName[20];
    int targetId;
    
    printf("\n>>> 预约挂号 <<<\n");
    printf("请输入您的姓名: ");
    scanf("%s", patientName);
    clearInputBuffer();
    
    printf("请输入您想预约的医生编号(ID): ");
    if (scanf("%d", &targetId) != 1) {
        printf("错误：医生ID应为数字！\n");
        clearInputBuffer();  // 清理输入缓冲区，防止残留字符影响后续输入
        return;
    }

    for (int i = 0; i < docCount; i++) {  // 遍历医生数组，查找与输入ID匹配的医生记录
        if (hospitalData[i].id == targetId) {
            if (hospitalData[i].current_num >= hospitalData[i].max_num) {
            	//校验1：号源充足性检查 判断当前已预约数是否达到或超过号源上限
                printf("\n【预约失败】%s 医生的号源已全部约满！\n", hospitalData[i].name);
                return;
            }
            
            for (int j = 0; j < recordCount; j++) {
            	//校验2：防重复预约检查  遍历历史预约记录，检查该患者是否已预约该医生
                if (records[j].doctorId == targetId && strcmp(records[j].patientName, patientName) == 0 && records[j].status == 0) {
                	// 三重条件判断：同医生ID + 同患者姓名 + 状态为有效
                    printf("\n【提示】您已预约过该医生，请勿重复预约！\n");
                    return;
                }
            }
            
            hospitalData[i].current_num++;   // 校验通过，递增医生的当前预约数（占用一个号源）
            
            Appointment newRecord; // 定义新的预约记录结构体变量
            newRecord.id = nextRecordId++;  // 分配预约编号：使用全局计数器nextRecordId，使用后自增  确保编号全局唯一且连续
            strcpy(newRecord.patientName, patientName);// 复制患者姓名至预约记录
            newRecord.doctorId = targetId; // 记录医生ID
            strcpy(newRecord.doctorName, hospitalData[i].name);
            strcpy(newRecord.dept, hospitalData[i].dept);
            newRecord.sequenceNum = hospitalData[i].current_num;  // 分配就诊序号：即该医生当前预约数（递增后的值） 例如：递增后current_num为6，则就诊序号为6号
            newRecord.status = 0;  // 设置预约状态：0表示有效预约
            getCurrentTime(newRecord.appointmentTime); // 获取当前系统时间，格式为"YYYY-MM-DD HH:MM"
            
            records[recordCount++] = newRecord;  // 将新记录追加至预约记录数组尾部
            
            printf("\n╔══════════════════════════════════════╗\n");
            printf("║          【预约成功】                ║\n");
            printf("╠══════════════════════════════════════╣\n");
            printf("║ 预约编号: %-4d                       ║\n", newRecord.id);
            printf("║ 患者姓名: %-10s                   ║\n", patientName);
            printf("║ 就诊医生: %-10s                   ║\n", hospitalData[i].name);
            printf("║ 就诊科室: %-14s               ║\n", hospitalData[i].dept);
            printf("║ 就诊序号: %-2d号                       ║\n", newRecord.sequenceNum);
            printf("║ 预约时间: %-19s      ║\n", newRecord.appointmentTime);
            printf("╚══════════════════════════════════════╝\n");
            printf("温馨提示：请牢记您的预约编号，就诊时请出示！\n");
            return;
        }
    }
    printf("\n【系统提示】未找到编号为 %d 的医生。\n", targetId);
}

void queryMyAppointment() {
    char patientName[20];
    printf("\n请输入您的姓名查询预约记录: ");
    scanf("%s", patientName);
    clearInputBuffer();
    
    int found = 0;
    printf("\n【%s】的预约记录：\n", patientName);
    printf("┌──────┬────────────┬────────────────┬──────┬────────────┬────────┐\n");
    printf("│编号  │ 医生姓名   │ 科室           │序号  │ 预约时间   │ 状态   │\n");
    printf("├──────┼────────────┼────────────────┼──────┼────────────┼────────┤\n");
    
    for (int i = 0; i < recordCount; i++) {  // 遍历预约记录数组，查找匹配该患者的记录  recordCount为全局变量，记录当前系统中预约记录总数
        if (strcmp(records[i].patientName, patientName) == 0 && records[i].status != 1) {    
            // 双重条件判断：患者姓名匹配 且 状态不为已取消（1） status != 1 过滤掉已取消的记录，仅显示有效和已完成记录
            found = 1;
            char statusStr[10];
            switch(records[i].status) {
                case 0: strcpy(statusStr, "有效"); break; // 状态0表示有效预约，可正常就诊

                default: strcpy(statusStr, "未知");  // 其他异常状态，标记为未知便于排查
            }
            
            printf("│%-6d│ %-10s │ %-14s │ %-4d │ %-10s │ %-6s │\n",
                   records[i].id, records[i].doctorName, records[i].dept,
                   records[i].sequenceNum, records[i].appointmentTime, statusStr);
        }
    }
    printf("└──────┴────────────┴────────────────┴──────┴────────────┴────────┘\n");
    
    if (!found) printf("未找到您的预约记录。\n");
}

void cancelAppointment() {
    int recordId;
    printf("\n请输入要取消的预约编号: ");
    if (scanf("%d", &recordId) != 1) {
        printf("错误：请输入数字编号！\n");
        clearInputBuffer();
        return;
    }
    
    for (int i = 0; i < recordCount; i++) {   // 遍历预约记录数组，查找匹配该患者的记录  
        if (records[i].id == recordId && records[i].status == 0) {
        	 // 双重检验：检验当前记录是否匹配：预约编号 且 状态为0（有效预约）
            for (int j = 0; j < docCount; j++) { // 【释放号源】遍历所有医生信息
    				if (hospitalData[j].id == records[i].doctorId) {
                    hospitalData[j].current_num--; // 该医生的当前预约人数减1（释放一个号源）
                    break;
                }
            }
            
            records[i].status = 1;
            printf("\n【取消成功】预约编号 %d 已取消，号源已释放。\n", recordId);
            return;
        }
    }
    printf("\n【错误】未找到该预约编号或该预约已失效。\n");
}

// ==================== 管理员功能 ====================

void adminMenu() {
    char pwd[20];
    printf("\n请输入管理员密码: ");
    scanf("%s", pwd);
    clearInputBuffer();
    
    if (strcmp(pwd, ADMIN_PWD) != 0) {
        printf("密码错误！\n");
        return;
    }
    
    int choice;
    while (1) {
        printf("\n╔════════════════════════════════╗\n");
        printf("║        【管理员控制台】         ║\n");
        printf("╠════════════════════════════════╣\n");
        printf("║ 1. 添加医生信息                ║\n");
        printf("║ 2. 删除医生信息                ║\n");
        printf("║ 3. 重置每日号源（清0预约数）   ║\n");
        printf("║ 0. 返回主菜单                  ║\n");
        printf("╚════════════════════════════════╝\n");
        printf("请选择: ");
        
        if (scanf("%d", &choice) != 1) {
            clearInputBuffer();
            continue;
        }
        
        switch (choice) {
            case 1: addDoctor(); break;
            case 2: deleteDoctor(); break;
            case 3: resetDailyQuota(); break;
            case 0: return;
            default: printf("无效选择。\n");
        }
    }
}

void addDoctor() {
    if (docCount >= 100) {
        printf("系统医生数量已达上限（100人）！\n");
        return;
    }
    
    Doctor newDoc;
    printf("\n请输入医生编号: ");
    scanf("%d", &newDoc.id);
    clearInputBuffer();
    
    for (int i = 0; i < docCount; i++) {
        if (hospitalData[i].id == newDoc.id) {
            printf("错误：该编号已存在！\n");
            return;
        }
    }
    
    printf("请输入医生姓名: ");
    scanf("%s", newDoc.name);
    printf("请输入所属科室: ");
    scanf("%s", newDoc.dept);
    printf("请输入每日限号数: ");
    scanf("%d", &newDoc.max_num);
    newDoc.current_num = 0;
    
    hospitalData[docCount++] = newDoc;
    printf("\n【成功】医生 %s 信息已添加！\n", newDoc.name);
}

void deleteDoctor() {
    int id;
    printf("\n请输入要删除的医生编号: ");
    scanf("%d", &id);
    
    for (int i = 0; i < docCount; i++) {
        if (hospitalData[i].id == id) {
            if (hospitalData[i].current_num > 0) {
                printf("该医生仍有 %d 个有效预约，无法删除！\n", hospitalData[i].current_num);
                return;
            }
            
            for (int j = i; j < docCount - 1; j++) {
                hospitalData[j] = hospitalData[j + 1];
            }
            docCount--;
            printf("医生编号 %d 已删除。\n", id);
            return;
        }
    }
    printf("未找到该医生。\n");
}

void resetDailyQuota() {
    char confirm;
    printf("\n警告：此操作将清空所有医生的预约数，是否继续？(y/n): ");
    scanf(" %c", &confirm);
    
    if (confirm == 'y' || confirm == 'Y') {
        for (int i = 0; i < docCount; i++) {
            hospitalData[i].current_num = 0;
        }
        for (int i = 0; i < recordCount; i++) {
            if (records[i].status == 0) {
                records[i].status = 2;
            }
        }
        printf("每日号源已重置，所有医生预约数清0。\n");
    }
}


// ==================== 工具函数 ====================

void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void getCurrentTime(char* buffer) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    sprintf(buffer, "%04d-%02d-%02d %02d:%02d",
            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
            t->tm_hour, t->tm_min);
}
