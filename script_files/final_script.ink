// TAG LIST:
// pause: stop until prompted to.
// battle: start the soul capture minigame when this dialogue closes.
// IDEAS: 
// can the patient talk to the player during minigame? 
VAR speaker = "Someone"
VAR start_abigail = false
VAR start_sherry = false
VAR start_keith = false
VAR start_david = false
VAR start_twins = false
VAR complete_abigail = false
VAR complete_sherry = false
VAR complete_keith = false
VAR complete_david = false
VAR complete_twins = false
VAR diary_henry = false
VAR diary_abigail = false
VAR diary_sherry = false
VAR diary_keith = false
VAR diary_david = false
VAR diary_twins = false
VAR battle_difficulty = 3


// -> henry


// === henry ===
//     ~ battle_difficulty = 3
   
//     ~ speaker = "Henry"
//     { diary_henry:
//         DIARY CONTENTS: 
//         "Stacy and the grandbabies visited me again. 
//         Jack turned 14 yesterday and he starts high school soon... 
//         Alice is entering the third grade and she adores her friends. 
//         They are both growing so big. Andrew couldn't visit because of his work. 
//         He should spend more time with them.
//         Stacy looks just like Dee when she was younger. 
//         If only Dee could see them now." 
//         ~ battle_difficulty = 1
//     }
//     * "It is time."
//     { diary_henry:
//             "'It is time'...? Ah, are you death? I can't go...
//             I am not ready. Everyone still needs me...
//             But you read my diary so I dont mind"
//     - else: 
//         "'It is time'...? Ah, are you death? I can't go...
//         I am not ready. Everyone still needs me...
//         But you didnt read my diary so I'll put up a fight!"
//     } # battle
    
// ~ speaker = "Someone"
// Congraulations on your first capture.
// Hopefully it wasn't that difficuly for you.
// I think that you'll do just fine out there.
// Good luck.





-> home 
=== home ===
{ complete_abigail && complete_keith && complete_sherry && complete_david && complete_twins:
    -> END
}
-> DONE

=== next_patient ===
{ 
    - start_abigail && not complete_abigail:
        -> abigail
    - start_keith && not complete_keith: 
        -> keith
    - start_sherry && not complete_sherry:
        -> sherry
    - start_david && not complete_david:
        -> david
    - start_twins && not complete_twins:
        -> twins
}


=== abigail ===
    ~ battle_difficulty = 3
    ~ speaker = "Notebook"

    { diary_abigail:
        DIARY CONTENTS:
        This hurts!
    }
    ~ speaker = "Abigail"

    * "It is time." 
    - "Time...? But I'm can't!"
    - "I havent finished my dissertation."
    - "I... I must finish. # battle
    ~ complete_abigail = true
    -> home
    
=== sherry ===
    ~ battle_difficulty = 4
    ~ speaker = "Notebook"

    { diary_sherry: 
        DIARY CONTENTS:
        I have fought a good fight, 
        I have finished my course, 
        I have kept the faith:
        henceforth there is laid up for me a crown of righteousness, 
        which the Lord, the righteous judge, 
        shall give me at that day: 
        and not to me only, 
        but unto all them also that love his appearing.
        \-- 2 Timothy 4:7-8.
        Every day I grow more weary. 
        It may be time for this ol' Sherry to join the rest of my friends.
    }
    ~ speaker = "Sherry"

    * "It is time." 
    - "Ahhh my Lord, my God. The Father, Son, and The Holy Spirit."
    - "Have you come to lead me to the afterlife?"
    * "One can say that."
    - "You... are not my savior."
    - "The way you are dressed..."
    - "... and that ferioucios weapon..!"
    - "You... must be Satan. God cannot look like this"
    * { diary_sherry } [You cannot see my face...] -> success
    * { diary_sherry } [The Lord may take many forms...] -> weak
    * { diary_sherry } [I am both, yet neither...] -> fail
    * { not diary_sherry } -> fight
    
    = success
        "Exodus 33:20... What the Lord said to Moses before he led the people to Isreal"
        "I understand that is is you my Lord,"
        "and that I was wrong to doubt you."
        ~ battle_difficulty = 1
        -> fight

    = weak
        "The form for me? That is peculiar."
        "But I shouldn't doubt God's plan..."
        ~ battle_difficulty = 3
        -> fight

        
    = fail
        "That is absolutely sacriligious."
        "Only a demon of Satan would conflate God and the Devil."
        ~ battle_difficulty = 5
        -> fight
    
    = fight
        Collection Complete # battle
        ~ complete_sherry = true
        -> home

=== keith ===
    ~ battle_difficulty = 1
    ~ speaker = "Notebook"

    { diary_keith:
        DIARY CONTENTS: 
        "finally an escape from my suffering
        an excuse from everything in my life
        if im here then everyone will be fine if i do nothing
        i dont need to worry
        i just wnat to sleep foreever
        i can finally rest
        im done 
        im done im doneimdoneimdone 
        imdoneimdoneimdoneiwanttodisappear
        its time to disappear"
        ~ battle_difficulty = 1
    }
    ~ speaker = "Keith"

    "...yea, sure, I'm fine with this."
    * "...W-what?"
    - "Youre Death right?"
    * "One can say that."
    - "Okay, perfect, sure, I'm good to go. So how does this thing work?"
    * "Well I peer into your soul, the thread that connects your conciousne--"
    - "Bro actually, I kinda dont gaf, can we just get this over with?"
    { diary_keith: 
        -> diary
        - else: 
        -> fight
    }
    = diary
    * "Whatever you wish."
        -> fight
    * "I can't reap you." 
        -> reject
    = fight 
        - Collection Complete # battle
        ~ complete_keith = true
        -> home

    = reject
        - "..."
        - "Am I really that pathetic that Death himself is rejecting me?"
        * "Well - no - you're just unwell, and need help and I cannot give you that."
        - "... how persumptuous of you to think you know what I need."
        - "Just leave me be."
        ~ complete_keith = true
        -> home


=== david ===
    ~ battle_difficulty = 2
    ~ speaker = "Notebook"

    { diary_david: 
       DIARY CONTENTS:
       I knew I shouldnt have drank and drove.
       I deserve this.
    }
    ~ speaker = "David"

    * "It is time." 
    - "Uwgh-- wah?"
    "Who're youse?"
    * "You must depart from this world"
    - "this aint an airport."
    { diary_david: 
        * "Your body is weak from poison, so this will be rather trivial."
            -> weak
        * "Im here to bring you to your gate, sir."
            -> success
        * "It's not, you are in a hospital."
            -> fail
        - else:
            -> fight
    }
    = success
        "You are?"
        "Perrrrrrrrfect lets fuckin go"
        "Im always ready for a time in Cancun"
        ~ battle_difficulty = 1
        -> fight

    = weak
        "Im not sick, im totally fine bro"
        "I can woalk it off."
        "But i may have totaled my car"
        "fuck"
        "IthinkINeedToFindMyCarToSeeIfITotalledIt."
        ~ battle_difficulty = 2
        -> fight

        
    = fail
        "Blech, hospials are so gross."
        "Reminds me of the time I had my stomach pumped."
        "I gotta get outta here"
        ~ battle_difficulty = 4
        -> fight
    
    = fight
         Collection Complete # battle
        ~ complete_david = true
        -> home
    

=== twins ===
    ~ battle_difficulty = 3
    ~ speaker = "Notebook"
    { diary_twins: 
        DIARY CONTENTS:
        \-- The diary is empty \--
    }
    
    * "It is time." 
    ~ speaker = "Luca"
    - "..."
    - "........"
    - "............."
    ~ speaker = "Matteo"
    - "Hello? Who're you?"
    * The one that will pass this one on to the next world.
        -> pass_on
    * You shouldn't concern yourself with me.
        -> shouldnt
   
    
    = pass_on
        - "Pass on?"
        - "..."
        - "Are you saying that he will die tonight?"
        * Yes.
            -> confirmation
        * It is inevitable.
            -> confirmation
        
    = confirmation
        "No!"
        "Who says he has to?"
        "Luca doesnt deserve this."
        "He's already done so much for me."
        "Ever since we were babies hes offered himself to me..."
        * How he's lived is quite irreleveant to me.
            -> cold
        * I see. 
            -> medium
        * A generous soul? A pity.
            -> warm
        
    = cold
        "But it matters to me!"
        * A human cannot interfere with my job.
            -> fight
    = medium
        "..."
        "I'm really sorry Luca."
        "Im sorry that I couldn't do anything."
        -> fight
    = warm
        "Yes..."
        "We were hiding in the closet."
        "Just waiting... for it to be finally over."
        "It was loud."
        "And there were so many screams." 
        "I...I couldn't breathe."
        "I was really scared, so he hugged me."
        "Ever since we were babies, Luca's hugs are the only thing that really comforts me."
        "They're big and warm, even though we are the same size."
        "When he hugs me, everything else slips away from my mind."
        "It's like we become our own cocoon."
        "Where we cannot see or hear anythign besides me and Luca."
        "I almost didn't notice when Mommy became quiet."
        "Haha..."
        "I didn't know how badly he was hurt until the nurses told me."
        "...I pleaded with the doctors to share a room with him.
        "Because I didn't want to leave his side."
        "..."
        "I think I know who you are."
        "You're Death, right?"
        "Ms. Robinson told us how some places in history had a personification of Death that brings them to the afterlife."
        "So are you here to take Luca?"
        "..."
        "I can't do anything without Luca."
        "He saved me."
        "And I kinda owe it to him."
        
        "...Can you take me instead?"
        * Yes.
            -> matteo
        * No.
            -> beg
         
    = matteo
        ~ battle_difficulty = 1
        "Really?"
        "Thanks."
        "I hope he won't be mad at me for this."
        -> fight
    = beg
        "Please?"
        * I really can't. I'm sorry. It's not up to me.
            -> luca
        * Well, maybe I can make something work.
            -> matteo
    
    = luca
        "Ah..."
        "Okay."
        "...Goodbye Luca."
        -> fight
        
        
    = shouldnt
        "Acutally I should, why are you talking to Luca?"
        "Don't bother him. He's tired."
        "He just needs to sleep."
        "...He's already done a lot for me tonight."
        * He will be sleeping forever.
            -> forever
        * A generous soul?  A pity
            -> warm
        

    = forever
        ~battle_difficulty = 5
        "Forever?"
        "But that doesn't make sense because everyone will wake up..."
        "..........eventually."
        "..."
        "Are you going to take my brother away?"
        "That's not fair."
        "You can't...just do that! To my brother!"
        "He doesn't deserve that." 
        "He's so nice and funny."
        "I love his hugs."
        "And how he gives me the bigger half of the cookie."
        "He's not that hurt, so you can wait."
        "..."
        "Oh, youre not waiting."
        "...Please don't do this."
        "I love my brother a lot."
        "I shouldn't have told him that I hated him yesterday."
        "I can't tell him that I'll miss him."
        "Please..."
        "LUCA PLEASE DON'T LEAVE ME...!"
        -> fight
        
    
    = fight
        Collection Complete # battle
        ~ complete_twins = true
        -> home



-> END
