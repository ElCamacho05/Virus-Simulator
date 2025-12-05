#include "interface.h"
#include <GL/glut.h>
#include <stdio.h>
#include <string.h>

#include "utils.h"
#include "Algorithms.h"
#include "DAO_General.h"
#include "dataRepository.h"
#include "Regions.h"
#include "Person.h"

#define X 0
#define Y 1

int totalDaysPaused = 0; // sum of all the time that the app were kept paused
double sttPausedSimAt;
double endPausedSimAt;

double percentageOfAffected = 0.10;

// searching people control variables
char person_to_find[20];
int is_searching = 0;
int iPtF = -1;
PERSON *personFound = NULL;

// Helper: Retrieves the PERSON_HISTORY structure for a person ID
PERSON_HISTORY* get_person_history(int person_id) {
    if (!GlobalData || !GlobalData->history_table) return NULL;
    
    // Hash lookup
    unsigned int index = hashFunction(person_id, PERSON_HASH_TABLE_SIZE); 
    HISTORY_NODE *current = GlobalData->history_table->table[index];

    while (current != NULL) {
        if (current->person_id == person_id) return &current->history;
        current = current->next;
    }
    return NULL;
}

// Maps HealthStatus enum value to a string
const char* statusToString(HealthStatus status) {
    switch (status) {
        case HEALTH: return "HEALTHY";
        case INFECTED: return "INFECTED";
        case IMMUNE: return "IMMUNE";
        case DEATH: return "DECEASED";
        case ISOLATED: return "ISOLATED";
        case VACCINATED: return "VACCINATED";
        case QUARANTINE: return "QUARANTINE";
        default: return "UNKNOWN";
    }
}

// Draws a panel in the bottom-left with the last 5 history entries of the selected person
void drawPersonHistoryPanel() {
    if (!personFound) return;
    
    PERSON_HISTORY *history = get_person_history(personFound->id);
    if (!history || history->entry_count == 0) return;

    int max_entries = 5;
    
    // Panel coordinates (Bottom Left Quadrant, adjusted from center (0,0))
    float panel_x = -350.0f; // Adjusted for a typical 500xAspect window
    float panel_y = -220.0f;
    float panel_width = 150.0f;
    float panel_height = 130.0f;
    float text_offset_x = 5.0f;
    float line_height = 20.0f;
    
    // Draw the box
    glColor4f(0.1f, 0.1f, 0.1f, 0.8f); // Dark background, translucent
    glBegin(GL_QUADS);
        glVertex2f(panel_x, panel_y);
        glVertex2f(panel_x + panel_width, panel_y);
        glVertex2f(panel_x + panel_width, panel_y + panel_height);
        glVertex2f(panel_x, panel_y + panel_height);
    glEnd();
    
    // Draw border
    glColor3f(1.0f, 1.0f, 1.0f); // White border
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(panel_x, panel_y);
        glVertex2f(panel_x + panel_width, panel_y);
        glVertex2f(panel_x + panel_width, panel_y + panel_height);
        glVertex2f(panel_x, panel_y + panel_height);
    glEnd();
    
    // Display Title (Centered) - Keep display text in Spanish as requested for UI elements
    char title_buffer[100];
    snprintf(title_buffer, 100, "Historial de %s (ID: %d)", personFound->name, personFound->id);
    glColor3f(1.0f, 1.0f, 0.0f); // Yellow text
    text(title_buffer, panel_x + panel_width / 2.0f, panel_y + panel_height - 15.0f);
    
    // Display entries (Last one first, descending)
    glColor3f(1.0f, 1.0f, 1.0f); // White text for entries
    
    for (int i = 0; i < max_entries; i++) {
        // Index calculation to show the last 5 entries (most recent first)
        int current_entry_index = history->entry_count - 1 - i;
        if (current_entry_index < 0) break; 
        
        DAILY_HISTORY_ENTRY *entry = &history->entries[current_entry_index];
        
        char line_buffer[100];
        // Use Spanish status string for the display
        const char *status_str = statusToString((HealthStatus)entry->status); 
        
        // Format: [Dia 100] STATUS - (Cepa 5)
        snprintf(line_buffer, 100, "[Dia %d] %s (Cepa %d)", 
                 entry->day, 
                 status_str, 
                 entry->strain_id);
                 
        // Position: Starts below the title, moving down
        float y_pos = panel_y + panel_height - 35.0f - (i * line_height);
        
        // Use glRasterPos2f directly for left alignment (x + offset)
        glPushMatrix();
            glColor3f(1.0f, 1.0f, 1.0f);
            glRasterPos2f(panel_x + text_offset_x, y_pos);
            for (int k = 0; k < strlen(line_buffer); k++) {
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, line_buffer[k]);
            }
        glPopMatrix();
    }
}

// Function definition


void init(int *argc, char *argv[]) {
    glutInit(argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Virus Simulator V2");

    /* Disable lighting for 2D rendering */
    glDisable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_NORMALIZE);
    glShadeModel(GL_FLAT);

    // glClearColor(0.0, 0.0, 0.0, 1.0); // black
    // glClearColor(1.0f, 1.0f, 1.0f, 0.0f); // white
    glClearColor(0.2, 0.2, 0.4, 1.0); // navy blue

    /* Graphics info */
    // const GLubyte* vendor = glGetString(GL_VENDOR);
    // const GLubyte* renderer = glGetString(GL_RENDERER);
    // const GLubyte* version = glGetString(GL_VERSION);
    // if (vendor && renderer && version) {
    //     printf("VENDOR: %s\n", vendor);
    //     printf("RENDERER: %s\n", renderer);
    //     printf("VERSION: %s\n", version);
    // }
    

    glutDisplayFunc(display);
    glutIdleFunc(idle);
    glutKeyboardFunc(keyboard);
    glutReshapeFunc(reshape);
    glutMainLoop();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.06, 0.02, 0.14, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Draw simulation elements (regions and persons)
    drawRegions(GlobalData);

    // Draw the history panel for the selected person
    drawPersonHistoryPanel(); 

    glutSwapBuffers();
}

// Draws a line between two people, representing the infection link
void drawInfectionLine(PERSON *p1, PERSON *p2) {
    glColor3f(1.0, 0.0, 0.0);
    glLineWidth(1.0);
    glBegin(GL_LINES);
        glVertex3f(p1->drawConf.pos[X], p1->drawConf.pos[Y], 0.0);
        glVertex3f(p2->drawConf.pos[X], p2->drawConf.pos[Y], 0.0);
    glEnd();
}

// Draws all regions and the individuals within them
void drawRegions(BIO_SIM_DATA *data) {
    REGION_HASH_TABLE *reg = data->regions_table;
    PERSON_HASH_TABLE *pop = data->persons_table;
    
    // Draw population (Individuals)
    float r = 0.0, g = 0.0, b = 0.0, alpha = 1.0;
    for (int i = 0; i< PERSON_HASH_TABLE_SIZE; i++) {
        PERSON_NODE *pN = pop->table[i];
        while (pN) {
            PERSON *p1 = &pN->data;
            PERSON *p2 = get_person_by_id(data, p1->infectedBy);
            
            // Draw infection line if the person was infected by another active spreader
            if (p1->infectedBy != -1 && p1->status != DEATH && p2 && p2->status == INFECTED) {
                drawInfectionLine(p1, p2);
            }
            
            // Set color based on status
            if (pN->data.status == HEALTH) {
                r = 0.0; g = 1.0, b = 0.0;
            }
            else if(pN->data.status == INFECTED) {
                r = 1.0; g = 0.0; b = 0.0;
            }
            else if(pN->data.status == IMMUNE) {
                r = 0.0; g = 0.0; b = 1.0;
            }
            else if(pN->data.status == DEATH) {
                r = 0.0; g = 0.0; b = 0.0;
            }

            // Draw border for ISOLATED
            else if(pN->data.status == ISOLATED) {
                glPushMatrix();
                    glTranslatef(pN->data.drawConf.pos[X], pN->data.drawConf.pos[Y], 0.0);
                    r = 1.0; g = 1.0, b = 0.0; // YELLOW border
                    circle(2.5, 36, r, g, b, alpha);
                    r = 0.0; g = 0.0; b = 1.0; // blue color for middle circle
                glPopMatrix();
            }

            // Draw border for VACCINATED
            else if(pN->data.status == VACCINATED) {
                glPushMatrix();
                    glTranslatef(pN->data.drawConf.pos[X], pN->data.drawConf.pos[Y], 0.0);
                    r = 0.0; g = 1.0, b = 0.0; // GREEN border
                    circle(2.5, 36, r, g, b, alpha);
                    r = 0.0; g = 0.0; b = 1.0; // blue color for middle circle
                glPopMatrix();
            }

            // Draw the person (center dot)
            glPushMatrix();
                glTranslatef(pN->data.drawConf.pos[X], pN->data.drawConf.pos[Y], 0.0);
                circle(3.0, 36, r, g, b, alpha);
            glPopMatrix();
            pN = pN->next;
        }
    }
    
    // Draw regions (Circles representing geographic areas)
    for (int i = 0; i< REGION_HASH_TABLE_SIZE; i++) {
        REGION_NODE *rN = reg->table[i];

        while (rN) {
            // Calculate infection ratio for color shading
            double infectionRatio = 0.0;
            if (rN->data.populationCount > 0) {
                infectionRatio = (double)rN->data.infectedCount / (double)rN->data.populationCount;
            }

            if (infectionRatio > 1.0) infectionRatio = 1.0;

            // Color scheme: Blends towards Red (R=1.0) as infectionRatio increases
            double r = 1.0;
            double g = 1.0 - infectionRatio; 
            double b = 1.0 - infectionRatio; 
            
            // Alpha (transparency) increases with infection ratio for visibility
            double alpha = 0.3 + (infectionRatio * 0.5);
            
            glPushMatrix();
                glTranslatef(rN->data.drawConf.pos[X], rN->data.drawConf.pos[Y], 0.0);
                // circle(rN->data.drawConf.radio, 36, 1.0, 1.0, 1.0, 1.0);
                glColor4f(r, g, b, alpha);
                circle(rN->data.drawConf.radio, 36, r, g, b, alpha);
            glPopMatrix();
            rN = rN->next;
        }
    }
}

// Placeholder for drawing the history panel (if implemented)
void drawHistorialPanel(BIO_SIM_DATA *data) {
    // HISTORY_HASH_TABLE *history = data->history_table;
}

// Function executed when no events are pending (main loop of the simulation)
void idle() {
    if (GlobalData && !pause) {
        updateTime();
        int actualDay = (int)(elapsedTime/1000/secondsPerDay);
        // Advance simulation by one day
        if (actualDay > simulation_day) {
            printf("=== SIMULATING DAY %d | ACTIVE INFECTED: %d | Fatalities: %d ===\n", actualDay, GlobalData->infectedCount, GlobalData->deathCount);
            simulation_day++;
            run_daily_simulation(GlobalData, simulation_day); 
        }
        
    }
    // Request redraw to update visualization
    glutPostRedisplay(); 
}

// Handles window resizing and maintains aspect ratio
 void reshape(int w, int h)
{
    if (h == 0) h = 1;
    
    glViewport(0, 0, (GLsizei) w, (GLsizei) h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    /* Maintain aspect ratio: calculate logical dimensions proportionally */
    float aspect = (float)w / (float)h;
    float logical_h = 500.0;  /* Base height */
    float logical_w = logical_h * aspect;  /* Width scales with aspect ratio */
    
    /* Orthographic projection centered at origin */
    gluOrtho2D(-logical_w/2.0, logical_w/2.0, -logical_h/2.0, logical_h/2.0);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// Handles keyboard input for controls and search
void keyboard(unsigned char key, int x, int y) {
    if (is_searching) {
        if (key == 27) { // ESC to cancel search
                is_searching = 0;
                person_to_find[0] = '\0'; // clear string
                iPtF = -1;
                printf("[ SEARCH CANCELED ]\n");
                return;
            }
        // Enter
        if (key == 13) {
            // Clean string and look for the person
            is_searching = 0;
            person_to_find[iPtF] = '\0';
            iPtF = 0;

            printf("[ SEARCHING PERSON... '%s' ] \n", person_to_find);
            int Pid = atoi(person_to_find);
            PERSON *found = get_person_by_id(GlobalData, Pid);
            if (found) {
                printf("[ PERSON '%s' FOUND! ] \n", person_to_find);
                printf("[ SHOWING PERSON'S HISTORY ] '%s'\n", person_to_find);
                person_to_find[0] = '\0';
                personFound = found;
            } else {
                printf("[ PERSON '%s' not found ]\n", person_to_find);
            }
        }
        // Backspace
        else if (key == 8) {
            if (iPtF > 0) {
                iPtF--;
                person_to_find[iPtF] = '\0';
            }
        }
        // Add word to the search string
        else if (iPtF < sizeof(person_to_find) - 1) {
            person_to_find[iPtF++] = key;
            person_to_find[iPtF] = '\0';
        }
        return;
    }
    else if (key == 'b' || key == 'B') {
        is_searching = 1;
        printf("[ INITIALIZING SEARCHING MODE ]\n");
    }

    if (key == 27)
        exit(0);
    if (key == 'p' || key == 'P'){
        pause = !pause;
    }
    if (key == 'i' || key == 'I') { // minimize risk by isolating population
        // double isolationPercentage = (double)GlobalData->infectedCount/(double)GlobalData->max_individuos;
        minimize_total_risk(GlobalData, percentageOfAffected, ISOLATED);
    }
    if (key == 'v' || key == 'V') { // minimize risk by vaccinating population
        // double vaccinationPercentage = (double)GlobalData->infectedCount/(double)GlobalData->max_individuos;
        minimize_total_risk(GlobalData, percentageOfAffected, VACCINATED);
    }
}