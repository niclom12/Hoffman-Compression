#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

int* encode(const char* input, int* out_len) {
    Node* list = create_list();
    int len = strlen(input);
    int* output = malloc(len * sizeof(int));
    if (!output) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }

    for (int i = 0; i < len; i++) {
        output[i] = move_to_front(&list, (unsigned char)input[i]);
    }
    free_list(list);
    *out_len = len;
    return output;
}

char* decode(const int* indices, int len) {
    Node* list = create_list();
    char* result = malloc((len + 1) * sizeof(char)); // + for null
    if (!result) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }

    for (int i = 0; i < len; i++) {
        result[i] = get_symbol(&list, indices[i]);
    }
    result[len] = '\0';

    free_list(list);
    return result;
}

int main() {
    const char* test_input = "annnvvvddddaaaasdsasdnb$aa";
    int encoded_len = 0;
    int* encoded = encode(test_input, &encoded_len);

    printf("Encoded:\n");
    for (int i = 0; i < encoded_len; i++) {
        printf("%d ", encoded[i]);
    }
    printf("\n");

    char* decoded = decode(encoded, encoded_len);
    printf("Decoded:\n%s\n", decoded);

    free(encoded);
    free(decoded);

    return 0;
}
