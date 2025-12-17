# A1 - O Arquivador VINAc

# Autoria do software

    Autor:      Jean Pablo Lopes Teixeira
    GRR:        20224382
    Turma:      BCC4 e IBM4
    Professor:  Diego Addan
 
# Arquivos e diretórios contidos no pacote

    lz.h: Biblioteca com os protótipos para as funções de 
    compressão com interface LZ77 coder/decoder.

    lz.c: Arquivo com as funções de lz.h implementadas, 
    sendo duas funções de compressão, uma de descompressão
    e mais funções auxiliares.

    options.h: Biblioteca com os protótipos para as funções
    do arquivador VINAc, com structs para dados de arquivos,
    além de funções auxiliares e também todas as demais 
    bibliotecas usadas pelos arquivos do programa modular.

    options.c: Arquivo com as funções de options.h implementadas, 
    sendo seis funções de opção do arquivador e três funções 
    auxiliares com utilização das Structs de options.h.

    vina.c: Arquivo principal do programa modular, responsável
    por receber os argumentos na execução do programa e direcionar
    para as opções correspondentes, dada uma manipulação do Archive.

    makefile: Ferramenta Make para compilar os módulos do programa
    e gerar um executável. Contém regras como all e clean. 

    testes: Diretório com uma sequência de arquivos com diferentes
    tipos e tamanhos que podem ser usados para testar as opções
    do programa modular.

# Algoritmos e estruturas de dados utilizados (Prototipadas em options.h)

    Struct DirEntry: Esta estrutura adota os campos solicitados pelo
    enunciado do trabalho para compor as propriedades dos membros, 
    sendo colocados em ordem para facilitar o entendimento e com os tipos
    que fazem mais sentido com cada propriedade, como char para o nome
    (caracteres), uid_t para UID (identificação do usuário), size_t para
    tamanhos (quantidade de bytes dos arquivos), time_t para data de 
    modificação (localtime dos membros quando manipulados), int para 
    ordem (índice no Archive) e long para offset (espaço maior para dizer 
    seu byte de início em Archive). 

    Struct MemberData: Esta estrutura auxiliar carrega o conteúdo dos 
    membros presentes em Archive, bem como suas propriedades existentes 
    na estrutura DirEntry. O conteúdo do membro é inserido em uma 
    variável unsigned char *, pois facilita o armazenamento dos membros 
    em um ponteiro que pode ser identificado , e carregado em memória 
    para manipulação.

    Função DirEntry load_archive_meta: Função responsável por abrir o 
    Archive e carregar as propriedades de seus membros. Foi pensado com 
    sistema de leitura binária do arquivo, seguida da busca do offset do 
    diretório presente em Archivo por uma variável do tipo uint64_t, com 
    exatamente 64 bits, que garante esse padrão até em outros sistemas 
    operacionais, caso o programa seja testado em outros tipos de SO. 
    Outro detalhe é que as propriedades são lidas membro a membro, 
    formando um vetor que facilita saber os dados característicos de cada 
    membro presente.

    Função void save_archive: Função que por padrão abre o Archive em 
    modo escrita binária, para poder gravar os dados dos membros 
    (DirEntry's) sequencialmente e então mandar o ponteiro no Archive 
    para o início do diretório. Essa abordagem foi feita para garantir a 
    ordem dos membros presentes a facilitar leituras futuras do mesmo 
    Archive, a partir do começo de seu diretório.

    Função MemberData load_archive_data: Função encarregada de criar um 
    vetor de conteúdos dos membros (MemberData) em memória e abrir o Archive 
    em modo leitura binária para poder copiar os conteúdos dos membros. Foi 
    pensado desta forma para obter os conteúdos em ordem do Archive, e poder 
    retornar para qualquer função uma estrutura MemberData com os dados 
    carregados e prontos para uso. 

    # Options como funções do tipo void: As funções responsáveis pelas 
    opções inseridas por comando foram pensadas como void, pois a lógica
    adotada foi de que o Archive seria modificado dentro das funções, e
    para poupar custos de variáveis sendo criadas dentro da main para 
    receber valores das funções, as próprias funções recebem os ponteiros
    dos argumentos para que seja possível modificar os valores do main
    dentro delas sem ser necessário o retorno de algum valor, salvo
    exceções para mensagens de erro e outras impressões.

    Função void option_ip_p: Função responsável pela inserção sem 
    compressão, foi desenvolvida com a ideia de carregar os metadados dos 
    membros, carregar os dados antigos do Archive, mesclar com os novos 
    dados de membros passados por parâmetro e salvar dentro do Archive. Esse 
    fluxo foi elaborado para ter um padrão de código que também possa ser 
    usado em outros cenários, como no caso de outras funções do programa, 
    além de funcionar muito bem para casos com arquivos menores, onde a 
    quantidade de bytes alocados, mesmo com testes chegando à casa dos 
    milhares, ainda assim não apresentava grande demora na execução.

    Função void option_ic_i: Função que segue os mesmos passos iniciais da 
    função de inserir sem compressao, porém desta vez ao chegar na fase de 
    anexar os novos membros em Archive, foi montado um fluxo para que se 
    tenha um buffer do arquivo com seu tamanho, em seguida um buffer de 
    saída com as especificações da função LZ (tamanho do arquivo*1.004+1) e 
    então uma tentativa de compressão com LZ_Compress, que caso 
    dê certo irá mandar o arquivo compactado para o archive, e caso 
    contrário será salvo o arquivo original. Este esquema de fluxo segue um 
    padrão comum a da função option_ip_p para tornar o código mais simples, 
    porém esta função tende a demandar um pouco mais em termos de execução 
    por causa da tentativa de compressão para cada arquivo.

    Função void option_r: Função encarregada por remover um ou mais membros 
    de Archive, que desta vez além de seguir com o padrão de carregar 
    membros em memória, utiliza uma técnica de filtragem que varre os 
    membros presentes em Archive e os compacta para esquerda do vetor que os 
    compõe, mantendo apenas os membros que não foram passados como argumento 
    da função. Ao final desse processo, é delimitado o novo tamanho do vetor 
    de membros e então são salvos os membros restantes em Archive. Foi 
    pensado nesta técnica para execução da função pois como os membros não 
    possuem uma ordenação explicíta dentro do Archive (apenas por ordem de 
    chegada), torna-se necessário realizar uma varredura para identificar os 
    membros que serão removidos e então mover os demais para suprir os 
    espaços remanescentes, bem como a exclusão completa do conteúdo do 
    membro marcado para não manter bytes inutilizáveis no Archive.

    Função void option_c: Função para imprimir os membros de Archive e suas 
    propriedades, conta com abertura do Archive e carregamento da sua área 
    de diretório para permitir a impressão, tanto do seu tamanho quanto das 
    características de seus membros. A função foi concebida com uma 
    impressão a mais do tamanho do Archive para possíveis testes, 
    verificando como se comporta o tamanho do Archive com certas inserções e 
    exclusões. Além disso, o formato de como são impressos os membros deixa 
    uma interface mais agradável para leitura das propriedades, bem como 
    segue a ordem do qual os membros foram inseridos, simbolizando um vetor 
    com elementos DirEntry's dispostos na vertical. 

    Função void option_x: Função que deve extrair membros especificados de 
    Archive, tem como seu fator distinto um laço de extração, que foi 
    elaborado com a ideia de percorrer o vetor de membros do Archive, 
    localizar os membros que foram especificados para extração (ou todos 
    caso não tenha sido especificado algum), e assim que os membros forem 
    sendo localizados, iniciar o processo de extração (escrever o arquivo 
    com conteúdo salvo no Archive), porém verificando antes se o membro está 
    comprimido, para que possa ser descomprimido primeiro e extraído/escrito 
    depois. Esse método foi utilizado para garantir uma extração mais segura 
    dos membros, e que também eles fiquem legíveis após a extração caso 
    estejam compactados em Archive.

    Função void option_m: Função de mover um membro especificado dentro da 
    ordem do Archive, possui uma complexidade maior dos que as funções 
    anteriores, sendo esta a última a ser desenvolvida para que se fosse 
    possível obter uma maior quantidade de recursos para se utilizar nesta 
    função. Sobre o fluxo desta função, inicialmente temos o carregamento de 
    dados dos membros em memória como padrão, seguido da busca pelos índices 
    do membro a ser movido e de seu target. Após esses fatores, temos a 
    parte mais complexa da função, que envolve reordenar o vetor de membros, 
    extraindo e guardando o membro a ser movido (com uso de memmove) e 
    copiar a memória do membro seguinte (à direita, caso exista) para fechar 
    a lacuna deixada pelo membro extraído. Assim que é feita essa 
    modificação no vetor de membros, verificamos para onde o membro será 
    movido (após target ou começo do vetor) e com base neste resultado, será 
    aberto espaço para inserir a cópia de dados do membro que foi extraído 
    para dentro do vetor de membros (outro uso de memmove) e então deslocar 
    os membros à direita. Por fim, atualizamos os conteúdos dos membros em 
    ram e salvamos dentro do Archive, concluindo o processo da função. Pela 
    dificuldade inicial de estrutura que esta função impôs, o fluxo descrito 
    mostrou-se o mais viável, com testes apresentando resultados 
    positivos mesmo com arquivos maiores.  

    # O programa principal vina.c: A função main foi pensada de forma 
    relativamente simples, pois com o intuito de mantê-la mais enxuta, foram 
    feitos sete condicionais, sendo o primeiro para verificar se o comando 
    de entrada para o programa está correto e os demais para cada uma das 
    seis opções presentes para o arquivador, deixando o código bem limpo e 
    de fácil compreensão. Para operar com os condicionais das opções, foram 
    criadas variáveis para receber os argumentos específicos da linha de 
    comando, e com a utilização de uma função que compara dois caracteres 
    (strcmp), podemos definir qual será a opção escolhida, para em seguida 
    chamar a função que realiza tal opção com os argumentos devidamente 
    encaminhados. Por fim, para cada condicional temos um return, para 
    garantir que o programa encerre após a chamada de sua função. 

# Bugs conhecidos

    Como o programa sempre foi testado com a utilização do Valgrin 
    (Diretivas --leak-check=full --show-leak-kinds=all --track-origins=yes), 
    a maior parte dos bugs apareceram nestes cenários, com alguns casos 
    isolados aparecendo já no momento de compilação dos arquivos. Dito isso, 
    dos bugs que apareceram durante o desenvolvimento (não estão em ordem 
    cronológica), temos os seguintes:

    # Bug de Loop infinito junto com "Invalid read" logo após a chamada de 
    LZ_Compress para comprimir blocos grandes: 
        
        Causa: Este bug dificultou bastante as tentativas iniciais de 
        compressão dos arquivos, pois caso o buffer de saída não for alocado 
        com tamanho suficiente, LZ_Compress irá um byte além do tamanho do 
        buffer, causando a leitura inválida, além que pela leitura errada, 
        um de seus laços não terá mais fim, causando o Loop infinito.

        Solução: Para dar fim a este problema, foi necessário, além de 
        interpretar corretamente a função LZ_Compress e usar (insize * 
        257) / 256 + 1 como parâmetro, também verificar sempre se insize 
        segue os parâmetros máximos que LZ_Compress é capaz de comprimir, 
        como sendo menor que 100000 e maior que 0 (arquivo com tamanho entre 
        esses valores), além de somar 1 byte e alocar insize + 1 no buffer 
        de entrada para eliminar a leitura inválida. 

    # Bugs de “Conditional jump or move depends on uninitialised value(s)”

        Causa: Durante o início do desenvolvimento, as leituras 
        experimentais com buffers de tamanho fixo permitiram que as funções 
        de inserir/substituir membros funcionassem perfeitamente. Porém, 
        adaptando os buffers para terem o tamanho do maior membro em 
        Archive, gerou vários casos em que Archive não tinha membros, e o 
        buffer tentava obter um tamanho que não existia, causando sérios 
        problemas com condicionais que dependiam desses buffers que não 
        possuem valores definidos.

        Solução: Inicializar os buffers com 0 ou valores equivalentes 
        antes de tentar a leitura dos Archives e também preencher DirEntry 
        com zeros, além de criar alguns verificadores a mais para ter 
        certeza que nenhum buffer assumiria um valor não inicializado.

    #  Bugs de “Invalid free() … free’d block” na função save_archive()

        Causa: Como está função auxiliar é chamada pela maior parte das 
        demais funções principais, foi complexo encontrar um padrão que se 
        encaixasse com todas elas, principalmente as funções principais que 
        já estavam 100% funcionais, pois em alguns cenários sempre haviam 
        chamadas duplicados de free() ou alguma liberação errada de 
        ponteiros.

        Solução: Além de remover trechos das funções principais prontas para 
        inserir a função save_archive(), foi necessário liberar apenas os 
        conteúdos de Structs como MemberData all, no laço em que ela libera 
        apenas all[i].data. Dessa forma, a própria função principal que 
        chamou save_archive() termina de liberar all por completo, sem risco 
        de existir um free duplo.
    
    # Bugs de "Segmentation Fault" em abertura de arquivos

        Causa: Na hora de fazer a abertura de um Archive ou um arquivo 
        passado por comando, não estava sendo tratado o cenário do arquivo 
        possuir algum problema ou falha para ser aberto (cenário em que o 
        arquivo era muito grande dava este bug também). Com isso, o programa 
        parava de rodar abruptamente e então apontava o erro de falha de 
        segmentação.

        Solução: Colocar verificadores em todos os cenários que os arquivos 
        eram abertos ajudou bastante a evitar estas falhas de segmentação, 
        bem como delimitar melhor o tamanho máximo de um membro suportado 
        pelo compressor para que a escrita não se tornasse inviável e o 
        Archive não fosse corrompido, causando após algum outro acesso um 
        bug de "Segmentation Fault".
    
    # Membros removidos deixando "sobras" de bytes em Archive

        Causa: Quando as funções de inserção foram desenvolvidas, os offsets 
        dos arquivos membros não foram bem preenchidos dentro de Archive, o 
        que ocasiounou problemas sérios no momento de implementação e testes 
        com a função de remoção, pois ela removia a DirEntry do membro 
        selecionado e parte do seus dados, porém como o começo deste arquivo 
        estava errado, a função de remoção eliminava parte do membro 
        especificado, e a partir dissos os demais ficavam corrompidos dentro 
        de Archive e o próprio Archive ficava com "sobras" desse membro 
        excluído.

        Solução: Até o ponto do problema existia apenas a Struct DirEntry 
        para os membros, porém como solução mais confiável para o problema, 
        foi criada a Struct MemberData, que é mais segura para conter 
        exatamente o conteúdo do membro, além do seu offset que estava em 
        DirEntry. Com isso, e com o uso correto da função fseek para 
        encontrar o offset do arquivo, ficou mais fácil salvar os dados dos 
        membros dentro do Archive e durante algumas manipulações como o caso 
        de remoção, trabalhar apenas com a Memberdata do(s) membro(s) a ser
        (em) removido(s) e depois reorganizar tudo no Archive para fechar as 
        lacunas de espaço.

    # Arquivos corrompidos em Archive após serem movidos

        Causa: Durante o desenvolvimento da função de mover membros, os 
        testes iniciais apontavam que sempre quando um membro era movido 
        para o posição de outro dentro do Archive, a função concluia seu 
        processo, mas assim que a impressão do Archive era feita, o membro 
        naquela ordem ficava com suas informações completamente corrompidas, 
        sendo inviável a impressão de suas propriedades para leitura.

        Solução: Para contornar o problema, foi necessário aprender uma nova 
        lógica de como deslocar os membros que serão movidos dentro de 
        Archive, e alterar os dados para que fique o mínimo de "sobras" de 
        membros possível em Archive. Com essa perspectiva foi utilizado a 
        função memmove para cópia de dados, pois só a manipulação de 
        ponteiros dentro da área de diretório se mostrou muito complexo para 
        resolver o bug. Com uso de memmove, surgiu a ideia de remover 
        temporariamente o membro que deveria ser movido, e só após isso ir 
        atrás do índice do target para delimitar aonde será feita a cópia do 
        membro movido em Archive, bem como o deslocamento dos demais membros 
        para direita, abrindo espaço para o membro movido ser colocado.

    A princípio estes foram todos os bugs mais aparentes durante o 
    desenvolvimento, tiveram alguns outros erros e warnings existentes, 
    porém a grande maioria deles foi por causa de pequenos descuidos com a
    sintaxe e casos de uso não previstos antes dos testes.